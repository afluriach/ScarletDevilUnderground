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

bool isRadarSensorType(GType type)
{
	return type == GType::enemySensor || type == GType::playerGrazeRadar;
}

b2BodyType getMassType(SpaceFloat mass)
{
	return static_cast<b2BodyType>(
		to_int(b2_staticBody)*bool_int(mass < 0.0) +
		to_int(b2_kinematicBody)*bool_int(mass == 0.0) +
		to_int(b2_dynamicBody)*bool_int(mass > 0.0)
	);
}

void ContactListener::BeginContact(b2Contact* contact)
{
	GObject* a = static_cast<GObject*>(contact->GetFixtureA()->GetUserData());
	GObject* b = static_cast<GObject*>(contact->GetFixtureB()->GetUserData());

	GType typeA = static_cast<GType>(contact->GetFixtureA()->GetFilterData().categoryBits);
	GType typeB = static_cast<GType>(contact->GetFixtureB()->GetFilterData().categoryBits);

	if (isRadarSensorType(typeA) && isRadarSensorType(typeB)) {
		log("sensors should not collide with each other");
		return;
	}
	else if (isRadarSensorType(typeA)) {
		RadarSensor* sensor = static_cast<RadarSensor*>(contact->GetFixtureA()->GetUserData());
		phys->sensorStart(sensor, b, contact);
		return;
	}
	else if (isRadarSensorType(typeB)) {
		RadarSensor* sensor = static_cast<RadarSensor*>(contact->GetFixtureB()->GetUserData());
		phys->sensorStart(sensor, a, contact);
		return;
	}

	if (typeA < typeB) {
		auto it = phys->beginContactHandlers.find(PhysicsImpl::collision_type(typeA, typeB));
		if (it != phys->beginContactHandlers.end()) {
			(phys->*(it->second))(a, b, contact);
		}
	}
	//If the same physics type is colliding, order doesn't matter
	else {
		auto it = phys->beginContactHandlers.find(PhysicsImpl::collision_type(typeB, typeA));
		if (it != phys->beginContactHandlers.end()) {
			(phys->*(it->second))(b, a, contact);
		}
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
	GObject* a = static_cast<GObject*>(contact->GetFixtureA()->GetUserData());
	GObject* b = static_cast<GObject*>(contact->GetFixtureB()->GetUserData());

	GType typeA = static_cast<GType>(contact->GetFixtureA()->GetFilterData().categoryBits);
	GType typeB = static_cast<GType>(contact->GetFixtureB()->GetFilterData().categoryBits);

	if (isRadarSensorType(typeA) && isRadarSensorType(typeB)) {
		log("sensors should not collide with each other");
		return;
	}
	else if (isRadarSensorType(typeA)) {
		RadarSensor* sensor = static_cast<RadarSensor*>(contact->GetFixtureA()->GetUserData());
		phys->sensorEnd(sensor, b, contact);
		return;
	}
	else if (isRadarSensorType(typeB)) {
		RadarSensor* sensor = static_cast<RadarSensor*>(contact->GetFixtureB()->GetUserData());
		phys->sensorEnd(sensor, a, contact);
		return;
	}

	if (typeA < typeB) {
		auto it = phys->endContactHandlers.find(PhysicsImpl::collision_type(typeA, typeB));
		if (it != phys->endContactHandlers.end()) {
			(phys->*(it->second))(a, b, contact);
		}
	}
	//If the same physics type is colliding, order doesn't matter
	else {
		auto it = phys->endContactHandlers.find(PhysicsImpl::collision_type(typeB, typeA));
		if (it != phys->endContactHandlers.end()) {
			(phys->*(it->second))(b, a, contact);
		}
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

#define _addHandler(a,b,begin,end) AddHandler(make_pair(GType::a, GType::b), &PhysicsImpl::begin,&PhysicsImpl::end)
#define _addHandlerNoEnd(a,b,begin) AddHandler(make_pair(GType::a, GType::b), &PhysicsImpl::begin,nullptr)
#define _addSensor(a,b) AddSensorHandler(GType::a, GType::b)

void PhysicsImpl::addCollisionHandlers()
{
	_addHandler(player, enemy, playerEnemyBegin, playerEnemyEnd);
	_addHandlerNoEnd(player, enemyBullet, playerEnemyBulletBegin);
	_addHandlerNoEnd(playerBullet, enemy, playerBulletEnemyBegin);
	_addHandlerNoEnd(playerBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(enemyBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(playerBullet, enemyBullet, bulletBulletBegin);
	_addHandlerNoEnd(player, foliage, playerFlowerBegin);
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

	collisionMasks.insert_or_assign(GType::playerGrazeRadar, to_uint(GType::enemyBullet));
	collisionMasks.insert_or_assign(
		GType::enemySensor,
		to_uint(GType::enemy) | to_uint(GType::enemyBullet) | to_uint(GType::player) | to_uint(GType::playerBullet)
	);
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

	if (type == GType::player || type == GType::enemy || type == GType::npc) {
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

void PhysicsImpl::logHandler(const string& base, b2Contact* contact)
{
    if(logPhysicsHandlers){
		GObject* a = static_cast<GObject*>(contact->GetFixtureA()->GetUserData());
		GObject* b = static_cast<GObject*>(contact->GetFixtureB()->GetUserData());

        log("%s: %s, %s", base.c_str(), a->getName(), b->getName());
    }
}

void PhysicsImpl::AddHandler(
	collision_type types,
	int(PhysicsImpl::*begin)(GObject*, GObject*, b2Contact*),
	void(PhysicsImpl::*end)(GObject*, GObject*, b2Contact*)
){
	auto actual = types;

	if (actual.first > actual.second)
		swap(actual.first, actual.second);

	if (begin)
		beginContactHandlers[actual] = begin;
	if (end)
		endContactHandlers[actual] = end;

	emplaceIfEmpty(collisionMasks, types.first, to_uint(0));
	emplaceIfEmpty(collisionMasks, types.second, to_uint(0));

	collisionMasks.at(types.first) |= to_uint(types.second);
	collisionMasks.at(types.second) |= to_uint(types.first);
}

void PhysicsImpl::logHandler(const string& name, GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s: %s, %s", name, a->getName(), b->getName());
}

int PhysicsImpl::playerEnemyBegin(GObject* a, GObject* b, b2Contact* arb)
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

    return 1;
}

void PhysicsImpl::playerEnemyEnd(GObject* a, GObject* b, b2Contact* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	Enemy* e = dynamic_cast<Enemy*>(b);

	if (p && e) {
		p->onEndTouchAgent(e);
		e->onEndTouchAgent(p);
	}
    
	logHandler("playerEnemyEnd", a,b);
}

int PhysicsImpl::playerEnemyBulletBegin(GObject* playerObj, GObject* bullet, b2Contact* contact)
{
    Player* player = dynamic_cast<Player*>(playerObj);
	Bullet* _bullet = dynamic_cast<Bullet*>(bullet);
	b2WorldManifold manifold;
	contact->GetWorldManifold(&manifold);

    if(logPhysicsHandlers)
        log("%s hit by %s", player->getName(), bullet->getName());

	if (player && _bullet) {
		_bullet->onAgentCollide(player, manifold.normal);
		player->onBulletCollide(_bullet);
	}

    return 1;
}

int PhysicsImpl::playerBulletEnemyBegin(GObject* a, GObject* b, b2Contact* contact)
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

    if(logPhysicsHandlers)
        log("%s hit by %s", b->getName(), a->getName());
    
    return 1;
}

int PhysicsImpl::bulletBulletBegin(GObject* a, GObject* b, b2Contact* arb)
{
	Bullet* _a = dynamic_cast<Bullet*>(a);
	Bullet* _b = dynamic_cast<Bullet*>(b);

	if (_a && _b) {
		_a->onBulletCollide(_b);
		_b->onBulletCollide(_a);
	}

	return 0;
}

int PhysicsImpl::playerFlowerBegin(GObject* a, GObject* b, b2Contact* arb)
{
    if(logPhysicsHandlers)
        log("%s stepped on", b->getName());
    
    return 1;
}

int PhysicsImpl::playerPickupBegin(GObject* a, GObject* b, b2Contact* arb)
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
    
    return 0;
}

int PhysicsImpl::bulletEnvironment(GObject* bullet, GObject* environment, b2Contact* contact)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	bool _sensor = environment->getBodySensor();
	b2Manifold* manifold = contact->GetManifold();

	if (_b && environment && !_sensor) {
		if (!_b->applyRicochet(manifold->localNormal)) {
			_b->onEnvironmentCollide(environment);

			if (auto _hs = dynamic_cast<Headstone*>(environment)) {
				_hs->hit(_b->getScaledDamageInfo().mag);
			}
		}
	}
    
    return 1;
}

int PhysicsImpl::bulletWall(GObject* bullet, GObject* wall, b2Contact* contact)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	Wall* _w = dynamic_cast<Wall*>(wall);
	bool _sensor = wall->getBodySensor();
	b2Manifold* manifold = contact->GetManifold();

	if (_b && _w && !_sensor) {
		if(!_b->applyRicochet(manifold->localNormal))
			_b->onWallCollide(_w);
	}

    return 1;
}

int PhysicsImpl::floorObjectBegin(GObject* floorSegment, GObject* obj, b2Contact* arb)
{
	FloorSegment* fs = dynamic_cast<FloorSegment*>(floorSegment);

	if (dynamic_cast<FloorSegment*>(obj)) {
		log("GSpace::floorObjectBegin: FloorSegment should not collide with another one.");
		return 0;
	}

	else if (!fs || !obj) {
		return 0;
	}

	else{
		obj->onContactFloorSegment(fs);
		return 1;
	}
}

void PhysicsImpl::floorObjectEnd(GObject* floorSegment, GObject* obj, b2Contact* arb)
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

int PhysicsImpl::playerAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerContact(p);
	}
	return 1;
}

void PhysicsImpl::playerAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerEndContact(p);
	}
}

int PhysicsImpl::enemyAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyContact(e);
	}
	return 1;
}

void PhysicsImpl::enemyAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyEndContact(e);
	}
}

int PhysicsImpl::npcAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb)
{
	Agent* npc = dynamic_cast<Agent*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (npc && as) {
		as->onNPCContact(npc);
	}
	return 1;
}

void PhysicsImpl::npcAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb)
{
	Agent* npc = dynamic_cast<Agent*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (npc && as) {
		as->onNPCEndContact(npc);
	}
}

int PhysicsImpl::environmentAreaSensorBegin(GObject* obj, GObject* areaSensor, b2Contact* arb)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectContact(obj);
	}

	return 1;
}

void PhysicsImpl::environmentAreaSensorEnd(GObject* areaSensor, GObject* obj, b2Contact* arb)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectEndContact(obj);
	}
}

int PhysicsImpl::sensorStart(RadarSensor* radar, GObject* target, b2Contact* arb)
{
	if (logPhysicsHandlers)
		log("radar sensed %s.", target->getName());
	radar->radarCollision(target);

	return 1;
}

void PhysicsImpl::sensorEnd(RadarSensor* radar, GObject* target, b2Contact* arb)
{
	if (logPhysicsHandlers)
		log("radar lost %s.", target->getName());
	radar->radarEndCollision(target);
}
