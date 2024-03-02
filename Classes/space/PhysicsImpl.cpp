//
//  PhysicsImpl.cpp
//  Koumachika
//
//  Created by Toni on 2/21/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "Bomb.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "FloorSegment.hpp"
#include "PhysicsImpl.hpp"
#include "Player.hpp"
#include "Sensor.hpp"
#include "Wall.hpp"

void sensorStart(Sensor* radar, GObject* target, b2Contact* arb);
void sensorEnd(Sensor* radar, GObject* target, b2Contact* arb);

template<typename T, typename U>
pair<T*, U*> getCastObjects(b2Contact* contact)
{
	GObject* a = contact->GetFixtureA()->GetCastUserData<GObject>();
	GObject* b = contact->GetFixtureB()->GetCastUserData<GObject>();

	return make_pair(
		dynamic_cast<T*>(a),
		dynamic_cast<U*>(b)
	);
}

pair<GObject*, GObject*> getObjects(b2Contact* contact)
{
	GObject* a = contact->GetFixtureA()->GetCastUserData<GObject>();
	GObject* b = contact->GetFixtureB()->GetCastUserData<GObject>();;

	return make_pair(a, b);
}

bool isReverseMatch(PhysicsImpl::collision_type pairA, PhysicsImpl::collision_type pairB)
{
	return pairA.first == pairB.second && pairA.second == pairB.first;
}

PhysicsImpl::collision_type getCanonicalPair(PhysicsImpl::collision_type in)
{
	auto result = in;
	if (result.first > result.second) {
		swap(result.first, result.second);
	}

	return result;
}

pair<GType, GType> getFixtureTypes(b2Contact* contact)
{
    GType typeA = getBaseType(static_cast<GType>(contact->GetFixtureA()->GetFilterData().categoryBits));
    GType typeB = getBaseType(static_cast<GType>(contact->GetFixtureB()->GetFilterData().categoryBits));

    return make_pair(typeA, typeB);
}

template <typename T, typename U>
PhysicsImpl::contact_func makeObjectPairFunc(void(*f)(T*, U*, b2Contact*), PhysicsImpl::collision_type types)
{
	return [f, types](b2Contact* contact) -> void {
		auto crntTypes = getFixtureTypes(contact);
		T* t = nullptr;
		U* u = nullptr;

		if (types == crntTypes) {
			tie(t, u) = getCastObjects<T, U>(contact);
		}
		else if (isReverseMatch(crntTypes, types)) {
			tie(u, t) = getCastObjects<U, T>(contact);
		}
		else {
			log2("Object pair function, unknown type pair 0x%x,0x%x!", to_int(crntTypes.first), to_int(crntTypes.second));
		}

		if (t && u) {
			f(t, u, contact);
		}
		else {
			if (!t)
				log1("Failed to cast first object to type %s!", typeid(T).name());
			if (!u)
				log1("Failed to cast second object to type %s!", typeid(U).name());
		}
	};
}

bool isRadarSensorType(GType type)
{
	return type == GType::agentSensor || type == GType::playerGrazeRadar;
}

bool isBulletType(GType type)
{
	return type == GType::enemyBullet || type == GType::playerBullet;
}

bool isAgentType(GType type)
{
	GType targetTypes = enum_bitwise_or3(GType, enemy, npc, player);

	return bitwise_and_bool(targetTypes, type);
}

b2BodyType getMassType(SpaceFloat mass)
{
	return static_cast<b2BodyType>(
		to_int(b2_staticBody)*bool_int(mass < 0.0) +
		to_int(b2_kinematicBody)*bool_int(mass == 0.0) +
		to_int(b2_dynamicBody)*bool_int(mass > 0.0)
	);
}

PhysicsImpl::contact_func makeSensorHandler(PhysicsImpl::sensor_func f, PhysicsImpl::collision_type types)
{
	return [f, types](b2Contact* contact) -> void {
		auto crntTypes = getFixtureTypes(contact);

		if (types == crntTypes) {
			auto sensor = contact->GetFixtureA()->GetCastUserData<Sensor>();
			auto object = contact->GetFixtureB()->GetCastUserData<GObject>();
			f(sensor, object, contact);
		}
		else if (isReverseMatch(crntTypes, types)) {
			auto sensor = contact->GetFixtureB()->GetCastUserData<Sensor>();
			auto object = contact->GetFixtureA()->GetCastUserData<GObject>();
			f(sensor, object, contact);
		}
	};
}

//By convention, it assumes the Bullet is object A in the callback.
//This means checking whether this is a reverse match according to fixture order.
SpaceVect getBulletNormal(b2Contact* contact)
{
	unsigned int typeB = contact->GetFixtureB()->GetFilterData().categoryBits;
	bool reverse = bitwise_and_bool(typeB, enum_bitwise_or(GType, playerBullet, enemyBullet));

	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	return manifold.normal * (reverse ? -1.0 : 1.0);
}

SpaceVect getAgentWallNormal(b2Contact* contact)
{
	unsigned int typeA = contact->GetFixtureB()->GetFilterData().categoryBits;
	bool reverse = bitwise_and_bool(typeA, GType::wall);

	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	return manifold.normal * (reverse ? -1.0 : 1.0);
}

#define _getTypes() tie(typeA,typeB) = getFixtureTypes(contact)

void ContactListener::BeginContact(b2Contact* contact)
{
	PhysicsImpl::collision_type types = getCanonicalPair( getFixtureTypes(contact) );

	auto it = phys->beginContactHandlers.find(types);
	if (it != phys->beginContactHandlers.end()) {
		(it->second)(contact);
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
	PhysicsImpl::collision_type types = getCanonicalPair( getFixtureTypes(contact) );

	auto it = phys->endContactHandlers.find(types);
	if (it != phys->endContactHandlers.end()) {
		(it->second)(contact);
	}
}

const unordered_map<GType, int> PhysicsImpl::typeGroups = {
	{ GType::bomb, 1 },
	{ GType::enemy, 1 },
	{ GType::environment, 1 },
	{ GType::npc, 1 },
	{ GType::player, 1 },
	{ GType::wall, 1 }
};

int PhysicsImpl::getGroup(GType type)
{
	return getOrDefault(typeGroups, type, 0);
}

PhysicsImpl::PhysicsImpl(GSpace* space) :
	gspace(space),
	world(space->world)
{
	contactListener = make_unique<ContactListener>(this);
	space->world->SetContactListener(contactListener.get());
}

#define _addHandler(a,b,begin,end) \
AddHandler( \
	make_pair(GType::a, GType::b), \
	makeObjectPairFunc(&begin, make_pair(GType::a, GType::b)), \
	makeObjectPairFunc(&end, make_pair(GType::a, GType::b)) \
)

#define _addHandlerNoEnd(a,b,begin) \
AddHandler( \
	make_pair(GType::a, GType::b), \
	makeObjectPairFunc(&begin, make_pair(GType::a, GType::b)), \
	nullptr \
)

#define _addSensorHandler(a,b) \
AddHandler( \
	make_pair(GType::a, GType::b), \
	makeSensorHandler(&sensorStart, make_pair(GType::a, GType::b)), \
	makeSensorHandler(&sensorEnd, make_pair(GType::a, GType::b)) \
)

#define _collide(a,b) addCollide(GType::a, GType::b);

pair<b2Body*, b2Fixture*> PhysicsImpl::createCircleBody(
	const SpaceVect& center,
	SpaceFloat radius,
	SpaceFloat mass,
	GType type,
	PhysicsLayers layers,
	bool sensor,
	any_ptr data
) {
	if (radius <= 0.0) {
		log0("invalid radius!");
		return make_pair(nullptr, nullptr);
	}

	if (!isValidType(type)) {
		log1("Invalid type %x!", to_int(getBaseType(type)));
		return make_pair(nullptr, nullptr);
	}


	b2Body* body;
	b2Fixture* shape;

	b2BodyDef def = generateBodyDef(type, center, mass);

	if (isAgentType(type)) {
		def.fixedRotation = true;
	}

	body = world->CreateBody(&def);
	body->SetUserData(data);

	if (mass > 0.0) {
		b2MassData massData{ mass,b2Vec2_zero, circleMomentOfInertia(mass,radius) };
		body->SetMassData(&massData);
	}

	b2CircleShape circle;
	circle.m_radius = radius;
	circle.m_p = b2Vec2_zero;

	b2FixtureDef fixture;
	fixture.userData = data;
	fixture.shape = &circle;
	fixture.isSensor = sensor;
	fixture.filter = generateFilter(type, layers);

	shape = body->CreateFixture(&fixture);

	return make_pair(body, shape);
}

pair<b2Body*, b2Fixture*> PhysicsImpl::createRectangleBody(
	const SpaceVect& center,
	const SpaceVect& dim,
	SpaceFloat mass,
	GType type,
	PhysicsLayers layers,
	bool sensor,
	any_ptr data
) {

	if (dim.x <= 0.0 || dim.y <= 0.0) {
		log0("invalid dimensions");
		return make_pair(nullptr, nullptr);
	}

	b2Body* body;
	b2Fixture* shape;

	b2BodyDef def = generateBodyDef(type, center, mass);
	body = world->CreateBody(&def);
	body->SetUserData(data);

	if (mass > 0.0) {
		b2MassData massData{ mass,b2Vec2_zero, rectangleMomentOfInertia(mass,dim) };
		body->SetMassData(&massData);
	}

	if (!isValidType(type)){
		log1("Invalid type 0x%x!", to_int(getBaseType(type)));
		return make_pair(nullptr, nullptr);
	}

	b2PolygonShape rect;
	rect.SetAsBox(dim.x * 0.5, dim.y * 0.5, b2Vec2_zero, 0.0);

	b2FixtureDef fixture;
	fixture.userData = data;
	fixture.shape = &rect;
	fixture.isSensor = sensor;
	fixture.filter = generateFilter(type, layers);

	shape = body->CreateFixture(&fixture);

	return make_pair(body, shape);
}

bool PhysicsImpl::isValidType(GType type) const
{
	return collisionMasks.find(getBaseType(type)) != collisionMasks.end();
}

b2Filter PhysicsImpl::generateFilter(GType type, PhysicsLayers layers)
{
	b2Filter filter;
	GType baseType = getBaseType(type);

	filter.categoryBits = to_uint(type);
	filter.maskBits = collisionMasks.at(baseType);
	filter.groupIndex = getGroup(baseType);
	filter.layers = to_uint(layers);

	return filter;
}

b2BodyDef PhysicsImpl::generateBodyDef(GType type, SpaceVect center, SpaceFloat mass)
{
	b2BodyDef def;

	def.type = getMassType(mass);
	def.position = toBox2D(center);
	def.angle = 0.0;
	def.bullet = isBulletType(type);

	return def;
}

const bool PhysicsImpl::logPhysicsHandlers = false;

const int PhysicsImpl::positionSteps = 12;
const int PhysicsImpl::velocitySteps = 12;

void PhysicsImpl::addCollide(GType a, GType b)
{
	emplaceIfEmpty(collisionMasks, a, to_uint(0));
	emplaceIfEmpty(collisionMasks, b, to_uint(0));

	collisionMasks.at(a) |= to_uint(b);
	collisionMasks.at(b) |= to_uint(a);
}

void PhysicsImpl::AddHandler(
	collision_type types,
	contact_func begin,
	contact_func end
){
	auto actual = types;
	if (actual.first > actual.second) {
		swap(actual.first, actual.second);
	}

	if (begin)
		beginContactHandlers[actual] = begin;
	if (end)
		endContactHandlers[actual] = end;

	addCollide(types.first, types.second);
}

void agentAgentBegin(Agent* a, Agent* b, b2Contact* arb)
{    
	a->onTouchAgent(b);
	b->onTouchAgent(a);
}

void agentAgentEnd(Agent* a, Agent* b, b2Contact* arb)
{
	a->onEndTouchAgent(b);
	b->onEndTouchAgent(a);
}

void agentBulletBegin(Agent* a, Bullet* b, b2Contact* contact)
{
	SpaceVect n = getBulletNormal(contact);

	b->onAgentCollide(a, -1.0 * n);
	a->onBulletCollide(b, n);
}

void bulletBulletBegin(Bullet* _a, Bullet* _b, b2Contact* arb)
{
	_a->onBulletCollide(_b);
	_b->onBulletCollide(_a);
}

void itemBegin(Player* p, GObject* inv, b2Contact* arb)
{
	inv->onPlayerContact(p);
}

void bulletEnvironment(Bullet* _b, GObject* environment, b2Contact* contact)
{
	bool _sensor = environment->getBodySensor();

	if (environment && !_sensor) {
		SpaceVect n = getBulletNormal(contact);
		if (!_b->applyRicochet(n)) {
			_b->onEnvironmentCollide(environment);

			environment->hit(_b->getScaledDamageInfo(), n);
		}
	}
}

void bulletWall(Bullet* _b, Wall* _w, b2Contact* contact)
{
	bool _sensor = _w->getBodySensor();

	if (!_sensor) {
		if(!_b->applyRicochet(getBulletNormal(contact)))
			_b->onWallCollide(_w);
	}
}

void agentWall(Agent* a, Wall* w, b2Contact* contact)
{
	SpaceVect n = getAgentWallNormal(contact);
	DamageInfo damage = w->getTouchDamage();
	
	if(damage.isNonzero()){
		a->hit(damage, n);
	}
}

void floorObjectBegin(FloorSegment* fs, GObject* obj, b2Contact* arb)
{
	obj->onContactFloor(fs);
}

void floorObjectEnd(FloorSegment* fs, GObject* obj, b2Contact* arb)
{
	obj->onEndContactFloor(fs);
}

void bombObjectBegin(Bomb* bomb, GObject* obj, b2Contact* arb)
{
	bomb->onTouch(obj);
}

void objectAreaSensorBegin(GObject* obj, AreaSensor* areaSensor, b2Contact* contact)
{
	areaSensor->beginContact(obj);
}

void objectAreaSensorEnd(GObject* obj, AreaSensor* areaSensor, b2Contact* contact)
{
	areaSensor->endContact(obj);
}

void sensorStart(Sensor* radar, GObject* target, b2Contact* arb)
{
	radar->collision(target);
}

void sensorEnd(Sensor* radar, GObject* target, b2Contact* arb)
{
	radar->endCollision(target);
}

void PhysicsImpl::addCollisionHandlers()
{
	_addSensorHandler(playerGrazeRadar, enemyBullet);
	_addSensorHandler(agentSensor, environment);
	_addSensorHandler(agentSensor, bomb);
	_addSensorHandler(agentSensor, enemy);
	_addSensorHandler(agentSensor, enemyBullet);
	_addSensorHandler(agentSensor, npc);
	_addSensorHandler(agentSensor, player);
	_addSensorHandler(agentSensor, playerBullet);

	_addHandler(player, enemy, agentAgentBegin, agentAgentEnd);
	_addHandlerNoEnd(player, enemyBullet, agentBulletBegin);
	_addHandlerNoEnd(enemy, playerBullet, agentBulletBegin);
	_addHandlerNoEnd(playerBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(enemyBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(playerBullet, enemyBullet, bulletBulletBegin);
	_addHandlerNoEnd(player, item, itemBegin);
	
	_addHandlerNoEnd(playerBullet, wall, bulletWall);
	_addHandlerNoEnd(enemyBullet, wall, bulletWall);
	_addHandlerNoEnd(player, wall, agentWall);
	_addHandlerNoEnd(enemy, wall, agentWall);

	_addHandler(player, areaSensor, objectAreaSensorBegin, objectAreaSensorEnd);
	_addHandler(enemy, areaSensor, objectAreaSensorBegin, objectAreaSensorEnd);
	_addHandler(npc, areaSensor, objectAreaSensorBegin, objectAreaSensorEnd);
	_addHandler(environment, areaSensor, objectAreaSensorBegin, objectAreaSensorEnd);
 
 	_addHandler(floorSegment, player, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, enemy, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, npc, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, bomb, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, environment, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, item, floorObjectBegin, floorObjectEnd);

 	_addHandlerNoEnd(bomb, player, bombObjectBegin);
 	_addHandlerNoEnd(bomb, enemy, bombObjectBegin);
 	_addHandlerNoEnd(bomb, npc, bombObjectBegin);
 	_addHandlerNoEnd(bomb, playerBullet, bombObjectBegin);
 	_addHandlerNoEnd(bomb, enemyBullet, bombObjectBegin);
 	_addHandlerNoEnd(bomb, bomb, bombObjectBegin);
 	_addHandlerNoEnd(bomb, environment, bombObjectBegin);
 	_addHandlerNoEnd(bomb, wall, bombObjectBegin);

	//Pitfall doesn't collide with anything, it is accessed only by query
    emplaceIfEmpty(collisionMasks, GType::pitfall, to_uint(0));
}
