//
//  PhysicsImpl.cpp
//  Koumachika
//
//  Created by Toni on 2/21/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "Collectibles.hpp"
#include "Enemy.hpp"
#include "EnemyBullet.hpp"
#include "EnvironmentalObjects.hpp"
#include "FloorSegment.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "InventoryObject.hpp"
#include "MapFragment.hpp"
#include "PhysicsImpl.hpp"
#include "Player.hpp"
#include "RadarSensor.hpp"
#include "Upgrade.hpp"
#include "Wall.hpp"

void playerEnemyBegin(GObject* a, GObject* b, b2Contact* arb);
void playerEnemyEnd(GObject* a, GObject* b, b2Contact* arb);
void playerEnemyBulletBegin(GObject* playerObj, GObject* bullet, b2Contact* arb);
void playerBulletEnemyBegin(GObject* a, GObject* b, b2Contact* arb);
void bulletBulletBegin(GObject* a, GObject* b, b2Contact* arb);
void playerPickupBegin(GObject* a, GObject* b, b2Contact* arb);
void bulletEnvironment(GObject* a, GObject* b, b2Contact* arb);
void bulletWall(GObject* bullet, GObject* unused, b2Contact* arb);
void floorObjectBegin(GObject* floorSegment, GObject* obj, b2Contact* arb);
void floorObjectEnd(GObject* floorSegment, GObject* obj, b2Contact* arb);
void playerAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb);
void playerAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb);
void enemyAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb);
void enemyAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb);
void npcAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb);
void npcAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb);
void environmentAreaSensorBegin(GObject* obj, GObject* areaSensor, b2Contact* arb);
void environmentAreaSensorEnd(GObject* obj, GObject* areaSensor, b2Contact* arb);

void sensorStart(RadarSensor* radar, GObject* target, b2Contact* arb);
void sensorEnd(RadarSensor* radar, GObject* target, b2Contact* arb);

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

PhysicsImpl::contact_func makeObjectPairFunc(PhysicsImpl::pairwise_obj_func f, PhysicsImpl::collision_type types)
{
	return [f, types](b2Contact* contact) -> void {
		auto crntTypes = getFixtureTypes(contact);
		PhysicsImpl::object_pair objects = getObjects(contact);

		if (types == crntTypes) {
			f(objects.first, objects.second, contact);
		}
		else if (isReverseMatch(crntTypes, types)) {
			f(objects.second, objects.first, contact);
		}
		else {
			log(
				"makeObjectPairFunc: called for invalid typepair %X,%X",
				to_uint(crntTypes.first),
				to_uint(crntTypes.second)
			);
		}
	};
}

bool isRadarSensorType(GType type)
{
	return type == GType::enemySensor || type == GType::playerGrazeRadar;
}

bool isBulletType(GType type)
{
	return type == GType::enemyBullet || type == GType::playerBullet;
}

bool isAgentType(GType type)
{
	return type == GType::enemy || type == GType::player || type == GType::npc;
}

b2BodyType getMassType(SpaceFloat mass)
{
	return static_cast<b2BodyType>(
		to_int(b2_staticBody)*bool_int(mass < 0.0) +
		to_int(b2_kinematicBody)*bool_int(mass == 0.0) +
		to_int(b2_dynamicBody)*bool_int(mass > 0.0)
	);
}

pair<GType, GType> getFixtureTypes(b2Contact* contact)
{
	GType typeA = static_cast<GType>(contact->GetFixtureA()->GetFilterData().categoryBits);
	GType typeB = static_cast<GType>(contact->GetFixtureB()->GetFilterData().categoryBits);

	return make_pair(typeA, typeB);
}

PhysicsImpl::contact_func makeSensorHandler(PhysicsImpl::radarsensor_func f, PhysicsImpl::collision_type types)
{
	return [f, types](b2Contact* contact) -> void {
		auto crntTypes = getFixtureTypes(contact);
		//Presumably, only one of these is a GObject.
		PhysicsImpl::object_pair objects = getObjects(contact);

		if (types == crntTypes) {
			auto sensor = static_cast<RadarSensor*>(contact->GetFixtureA()->GetUserData());
			f(sensor, objects.second, contact);
		}
		else if (isReverseMatch(crntTypes, types)) {
			auto sensor = static_cast<RadarSensor*>(contact->GetFixtureB()->GetUserData());
			f(sensor, objects.first, contact);
		}
	};
}

pair<GObject*, GObject*> getObjects(b2Contact* contact)
{
	GObject* a = static_cast<GObject*>(contact->GetFixtureA()->GetUserData());
	GObject* b = static_cast<GObject*>(contact->GetFixtureB()->GetUserData());

	return make_pair(a, b);
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

void PhysicsImpl::addCollisionHandlers()
{
	_addSensorHandler(playerGrazeRadar, enemyBullet);
	_addSensorHandler(enemySensor, bomb);
	_addSensorHandler(enemySensor, enemy);
	_addSensorHandler(enemySensor, enemyBullet);
	_addSensorHandler(enemySensor, player);
	_addSensorHandler(enemySensor, playerBullet);

	_addHandler(player, enemy, playerEnemyBegin, playerEnemyEnd);
	_addHandlerNoEnd(player, enemyBullet, playerEnemyBulletBegin);
	_addHandlerNoEnd(playerBullet, enemy, playerBulletEnemyBegin);
	_addHandlerNoEnd(playerBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(enemyBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(playerBullet, enemyBullet, bulletBulletBegin);
    _addHandlerNoEnd(player,playerPickup,playerPickupBegin);
	_addHandlerNoEnd(playerBullet, wall, bulletWall);
	_addHandlerNoEnd(enemyBullet, wall, bulletWall);    

	_addHandler(floorSegment, player, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, enemy, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, npc, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, environment, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, bomb, floorObjectBegin, floorObjectEnd);

	_addHandler(player, areaSensor, playerAreaSensorBegin, playerAreaSensorEnd);
	_addHandler(enemy, areaSensor, enemyAreaSensorBegin, enemyAreaSensorEnd);
	_addHandler(npc, areaSensor, npcAreaSensorBegin, npcAreaSensorEnd);
	_addHandler(environment, areaSensor, environmentAreaSensorBegin, environmentAreaSensorEnd);
}

pair<b2Body*, b2Fixture*> PhysicsImpl::createCircleBody(
	const SpaceVect& center,
	SpaceFloat radius,
	SpaceFloat mass,
	GType type,
	PhysicsLayers layers,
	bool sensor,
	void* obj
) {
	if (radius <= 0.0) {
		log("createCircleBody: invalid radius!");
		return make_pair(nullptr, nullptr);
	}

	b2Body* body;
	b2Fixture* shape;

	b2BodyDef def;
	def.type = getMassType(mass);
	def.position = toBox2D(center);
	def.angle = 0.0;
	def.bullet = isBulletType(type);

	if (isAgentType(type)) {
		def.fixedRotation = true;
	}

	body = world->CreateBody(&def);

	if (mass > 0.0) {
		b2MassData massData{ mass,b2Vec2_zero, circleMomentOfInertia(mass,radius) };
		body->SetMassData(&massData);
	}

	b2CircleShape circle;
	circle.m_radius = radius;
	circle.m_p = b2Vec2_zero;

	b2FixtureDef fixture;
	fixture.userData = obj;
	fixture.shape = &circle;
	fixture.isSensor = sensor;
	fixture.filter.categoryBits = to_uint(type);
	fixture.filter.maskBits = collisionMasks.at(type);
	fixture.filter.groupIndex = getGroup(type);
	fixture.filter.layers = to_uint(layers);

	shape = body->CreateFixture(&fixture);

	if (def.type == b2_staticBody && (type == GType::environment || type == GType::wall)) {
		gspace->addNavObstacle(center, SpaceVect(radius*2.0, radius*2.0));
	}

	return make_pair(body, shape);
}

pair<b2Body*, b2Fixture*> PhysicsImpl::createRectangleBody(
	const SpaceVect& center,
	const SpaceVect& dim,
	SpaceFloat mass,
	GType type,
	PhysicsLayers layers,
	bool sensor,
	void* obj
) {

	if (dim.x <= 0.0 || dim.y <= 0.0) {
		log("createRectangleBody: invalid dimensions");
		return make_pair(nullptr, nullptr);
	}

	b2Body* body;
	b2Fixture* shape;

	b2BodyDef def;
	def.type = getMassType(mass);
	def.position = toBox2D(center);
	def.angle = 0.0;
	def.bullet = isBulletType(type);

	body = world->CreateBody(&def);

	if (mass > 0.0) {
		b2MassData massData{ mass,b2Vec2_zero, rectangleMomentOfInertia(mass,dim) };
		body->SetMassData(&massData);
	}

	b2PolygonShape rect;
	rect.SetAsBox(dim.x * 0.5, dim.y * 0.5, b2Vec2_zero, 0.0);

	b2FixtureDef fixture;
	fixture.userData = obj;
	fixture.shape = &rect;
	fixture.isSensor = sensor;
	fixture.filter.categoryBits = to_uint(type);
	fixture.filter.maskBits = collisionMasks.at(type);
	fixture.filter.groupIndex = getGroup(type);
	fixture.filter.layers = to_uint(layers);

	shape = body->CreateFixture(&fixture);

	if (mass < 0.0 && (type == GType::environment || type == GType::wall)) {
		gspace->addNavObstacle(center, dim);
	}


	return make_pair(body, shape);
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

void playerEnemyBegin(GObject* a, GObject* b, b2Contact* arb)
{    
    Player* p = dynamic_cast<Player*>(a);
    Enemy* e = dynamic_cast<Enemy*>(b);
    
    if(!p)
        log("%s is not a Player", a->getName().c_str());
    if(!e)
        log("%s is not an Enemy", b->getName().c_str());
    
	if (p && e) {
		p->onTouchAgent(e);
		e->onTouchAgent(p);
	}
}

void playerEnemyEnd(GObject* a, GObject* b, b2Contact* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	Enemy* e = dynamic_cast<Enemy*>(b);

	if (p && e) {
		p->onEndTouchAgent(e);
		e->onEndTouchAgent(p);
	}    
}

void playerEnemyBulletBegin(GObject* playerObj, GObject* bullet, b2Contact* contact)
{
    Player* player = dynamic_cast<Player*>(playerObj);
	Bullet* _bullet = dynamic_cast<Bullet*>(bullet);
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

	if (player && _bullet) {
		_bullet->onAgentCollide(player, manifold.normal);
		player->onBulletCollide(_bullet);
	}
}

void playerBulletEnemyBegin(GObject* a, GObject* b, b2Contact* contact)
{    
    Bullet* bullet = dynamic_cast<Bullet*>(a);
    Agent* _enemy_agent = dynamic_cast<Agent*>(b);
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

    if(!bullet)
        log("%s is not a Bullet", a->getName().c_str());
    if(!_enemy_agent)
        log("%s is not an Enemy", b->getName().c_str());
    
	if (bullet && _enemy_agent){
		bullet->onAgentCollide(_enemy_agent, manifold.normal);
		_enemy_agent->onBulletCollide(bullet);
	}
}

void bulletBulletBegin(GObject* a, GObject* b, b2Contact* arb)
{
	Bullet* _a = dynamic_cast<Bullet*>(a);
	Bullet* _b = dynamic_cast<Bullet*>(b);

	if (_a && _b) {
		_a->onBulletCollide(_b);
		_b->onBulletCollide(_a);
	}
}

void playerPickupBegin(GObject* a, GObject* b, b2Contact* arb)
{
    Player* p = dynamic_cast<Player*>(a);
    
    if(auto c = dynamic_cast<Collectible*>(b)){
        p->onCollectible(c);
    }
	else if (auto u = dynamic_cast<Upgrade*>(b)) {
		p->applyUpgrade(u);
	}
	else if (auto inv = dynamic_cast<InventoryObject*>(b)) {
		inv->onPlayerContact();
	}
	else if (auto _map = dynamic_cast<MapFragment*>(b)) {
		_map->onAcquire();
	}
}

void bulletEnvironment(GObject* bullet, GObject* environment, b2Contact* contact)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	bool _sensor = environment->getBodySensor();
	b2Manifold* manifold = contact->GetManifold();

	if (_b && environment && !_sensor) {
		if (!_b->applyRicochet(-1.0 * manifold->localNormal)) {
			_b->onEnvironmentCollide(environment);

			if (auto _hs = dynamic_cast<Headstone*>(environment)) {
				_hs->hit(_b->getScaledDamageInfo().mag);
			}
		}
	}
}

void bulletWall(GObject* bullet, GObject* wall, b2Contact* contact)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	Wall* _w = dynamic_cast<Wall*>(wall);
	bool _sensor = wall->getBodySensor();
	b2Manifold* manifold = contact->GetManifold();

	if (_b && _w && !_sensor) {
		if(!_b->applyRicochet(-1.0 * manifold->localNormal))
			_b->onWallCollide(_w);
	}
}

void floorObjectBegin(GObject* floorSegment, GObject* obj, b2Contact* arb)
{
	FloorSegment* fs = dynamic_cast<FloorSegment*>(floorSegment);

	if (dynamic_cast<FloorSegment*>(obj)) {
		log("GSpace::floorObjectBegin: FloorSegment should not collide with another one.");
	}
	else if (!fs || !obj) {
	}
	else{
		obj->onContactFloorSegment(fs);
	}
}

void floorObjectEnd(GObject* floorSegment, GObject* obj, b2Contact* arb)
{
	FloorSegment* fs = dynamic_cast<FloorSegment*>(floorSegment);

	if (dynamic_cast<FloorSegment*>(obj)) {
		log("GSpace::floorObjectEnd: FloorSegment should not collide with another one.");
		return;
	}

	else if (!fs || !obj) {
		return;
	}

	else {
		obj->onEndContactFloorSegment(fs);
	}
}

void playerAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerContact(p);
	}
}

void playerAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerEndContact(p);
	}
}

void enemyAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyContact(e);
	}
}

void enemyAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyEndContact(e);
	}
}

void npcAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb)
{
	Agent* npc = dynamic_cast<Agent*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (npc && as) {
		as->onNPCContact(npc);
	}
}

void npcAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb)
{
	Agent* npc = dynamic_cast<Agent*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (npc && as) {
		as->onNPCEndContact(npc);
	}
}

void environmentAreaSensorBegin(GObject* obj, GObject* areaSensor, b2Contact* arb)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectContact(obj);
	}
}

void environmentAreaSensorEnd(GObject* areaSensor, GObject* obj, b2Contact* arb)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectEndContact(obj);
	}
}

void sensorStart(RadarSensor* radar, GObject* target, b2Contact* arb)
{
	radar->radarCollision(target);
}

void sensorEnd(RadarSensor* radar, GObject* target, b2Contact* arb)
{
	radar->radarEndCollision(target);
}
