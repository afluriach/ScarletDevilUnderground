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
#include "Upgrade.hpp"
#include "Wall.hpp"

PhysicsImpl::PhysicsImpl(GSpace* space) :
	gspace(space),
	physicsSpace(space->space)
{}

int PhysicsImpl::beginContact(cpArbiter* arb, cpSpace* space, void* data)
{
	OBJS_FROM_ARB;

	PhysicsImpl* _this = static_cast<PhysicsImpl*>(data);

	GType typeA = static_cast<GType>(arb->a_private->collision_type);
	GType typeB = static_cast<GType>(arb->b_private->collision_type);

	auto it = _this->beginContactHandlers.find(collision_type(typeA, typeB));

	if (it == _this->beginContactHandlers.end()) {
		it = _this->beginContactHandlers.find(collision_type(typeB, typeA));
		swap(a, b);
	}

	//No collide;
	if (it == _this->beginContactHandlers.end())
		return 0;

	if (a && b && it->second) {
		int(PhysicsImpl::*begin_method)(GObject*, GObject*, cpArbiter*) = it->second;
		return (_this->*begin_method)(a, b, arb);
	}

	return 1;
}

void PhysicsImpl::endContact(cpArbiter* arb, cpSpace* space, void* data)
{
	OBJS_FROM_ARB;

	PhysicsImpl* _this = static_cast<PhysicsImpl*>(data);

	GType typeA = static_cast<GType>(arb->a_private->collision_type);
	GType typeB = static_cast<GType>(arb->b_private->collision_type);

	auto it = _this->endContactHandlers.find(collision_type(typeA, typeB));

	if (it == _this->endContactHandlers.end()) {
		it = _this->endContactHandlers.find(collision_type(typeB, typeA));
		swap(a, b);
	}

	if (it == _this->endContactHandlers.end())
		return;

	if (a && b && it->second) {
		void(PhysicsImpl::*end_method)(GObject*, GObject*, cpArbiter* arb) = it->second;
		(_this->*end_method)(a, b, arb);
	}
}

#define _addHandler(a,b,begin,end) AddHandler<GType::a, GType::b>(&PhysicsImpl::begin,&PhysicsImpl::end)
#define _addHandlerNoEnd(a,b,begin) AddHandler<GType::a, GType::b>(&PhysicsImpl::begin,nullptr)

void PhysicsImpl::addCollisionHandlers()
{
	_addHandler(player, enemy, playerEnemyBegin, playerEnemyEnd);
	_addHandlerNoEnd(player, enemyBullet, playerEnemyBulletBegin);
	_addHandler(playerGrazeRadar, enemyBullet, playerGrazeRadarBegin,playerGrazeRadarEnd);
	_addHandlerNoEnd(playerBullet, enemy, playerBulletEnemyBegin);
	_addHandlerNoEnd(playerBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(enemyBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(playerBullet, foliage, noCollide);
	_addHandlerNoEnd(enemyBullet, foliage, noCollide);
	_addHandlerNoEnd(playerBullet, enemyBullet, bulletBulletBegin);
	_addHandlerNoEnd(player, foliage, playerFlowerBegin);
    _addHandlerNoEnd(player,playerPickup,playerPickupBegin);
    _addHandlerNoEnd(player,npc,collide);
	_addHandlerNoEnd(playerBullet, wall, bulletWall);
	_addHandlerNoEnd(enemyBullet, wall, bulletWall);    

	_addHandlerNoEnd(enemy, enemy, collide);
	_addHandlerNoEnd(environment, environment, collide);
	_addHandlerNoEnd(npc, npc, collide);

	_addHandler(enemySensor, player, sensorStart, sensorEnd);
	_addHandler(enemySensor, playerBullet, sensorStart, sensorEnd);
	_addHandler(enemySensor, enemy, sensorStart, sensorEnd);
	_addHandler(enemySensor, bomb, sensorStart, sensorEnd);

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

const bool PhysicsImpl::logPhysicsHandlers = false;

void PhysicsImpl::logHandler(const string& base, cpArbiter* arb)
{
    if(logPhysicsHandlers){
        OBJS_FROM_ARB
        
        log("%s: %s, %s", base.c_str(), a->name.c_str(), b->name.c_str());
    }
}

void PhysicsImpl::logHandler(const string& name, GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s: %s, %s", name.c_str(), a->name.c_str(), b->name.c_str());
}

int PhysicsImpl::playerEnemyBegin(GObject* a, GObject* b, cpArbiter* arb)
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

void PhysicsImpl::playerEnemyEnd(GObject* a, GObject* b, cpArbiter* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	Enemy* e = dynamic_cast<Enemy*>(b);

	if (p && e) {
		p->onEndTouchAgent(e);
		e->onEndTouchAgent(p);
	}
    
	logHandler("playerEnemyEnd", a,b);
}

int PhysicsImpl::playerEnemyBulletBegin(GObject* playerObj, GObject* bullet, cpArbiter* arb)
{
    Player* player = dynamic_cast<Player*>(playerObj);
	Bullet* _bullet = dynamic_cast<Bullet*>(bullet);

    if(logPhysicsHandlers)
        log("%s hit by %s", player->name.c_str(), bullet->name.c_str());

	if (player && _bullet) {
		_bullet->onAgentCollide(player, cpArbiterGetNormal(arb, 0));
		player->onBulletCollide(_bullet);
	}

    return 1;
}

int PhysicsImpl::playerGrazeRadarBegin(GObject* playerRadar, GObject* bullet, cpArbiter* arb)
{
	Player* player = dynamic_cast<Player*>(playerRadar);
	Bullet* _bullet = dynamic_cast<Bullet*>(bullet);

	if (player && _bullet) {
		player->onGrazeTouch(_bullet);
	}

	return 1;
}

void PhysicsImpl::playerGrazeRadarEnd(GObject* playerRadar, GObject* bullet, cpArbiter* arb)
{
	Player* player = dynamic_cast<Player*>(playerRadar);
	Bullet* _bullet = dynamic_cast<Bullet*>(bullet);

	if (player && _bullet) {
		player->onGrazeCleared(_bullet);
	}
}

int PhysicsImpl::playerBulletEnemyBegin(GObject* a, GObject* b, cpArbiter* arb)
{    
    Bullet* bullet = dynamic_cast<Bullet*>(a);
    Agent* _enemy_agent = dynamic_cast<Agent*>(b);
    
    if(!bullet)
        log("%s is not a Bullet", a->getName().c_str());
    if(!_enemy_agent)
        log("%s is not an Enemy", b->getName().c_str());
    
	if (bullet && _enemy_agent){
		bullet->onAgentCollide(_enemy_agent, cpArbiterGetNormal(arb, 0));
		_enemy_agent->onBulletCollide(bullet);
	}

    if(logPhysicsHandlers)
        log("%s hit by %s", b->name.c_str(), a->name.c_str());
    
    return 1;
}

int PhysicsImpl::bulletBulletBegin(GObject* a, GObject* b, cpArbiter* arb)
{
	Bullet* _a = dynamic_cast<Bullet*>(a);
	Bullet* _b = dynamic_cast<Bullet*>(b);

	if (_a && _b) {
		_a->onBulletCollide(_b);
		_b->onBulletCollide(_a);
	}

	return 0;
}

int PhysicsImpl::playerFlowerBegin(GObject* a, GObject* b, cpArbiter* arb)
{
    if(logPhysicsHandlers)
        log("%s stepped on", b->name.c_str());
    
    return 1;
}

int PhysicsImpl::playerPickupBegin(GObject* a, GObject* b, cpArbiter* arb)
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

int PhysicsImpl::bulletEnvironment(GObject* bullet, GObject* environment, cpArbiter* arb)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	bool _sensor = cpShapeGetSensor(environment->bodyShape);

	if (_b && environment && !_sensor) {
		if (!_b->applyRicochet(cpArbiterGetNormal(arb, 0))) {
			_b->onEnvironmentCollide(environment);

			if (auto _hs = dynamic_cast<Headstone*>(environment)) {
				_hs->hit(_b->getScaledDamageInfo().mag);
			}
		}
	}
    
    return 1;
}

int PhysicsImpl::noCollide(GObject* a, GObject* b, cpArbiter* arb)
{
    return 0;
}

int PhysicsImpl::collide(GObject* a, GObject* b, cpArbiter* arb)
{
    return 1;
}

int PhysicsImpl::bulletWall(GObject* bullet, GObject* wall, cpArbiter* arb)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	Wall* _w = dynamic_cast<Wall*>(wall);
	bool _sensor = cpShapeGetSensor(wall->bodyShape);

	if (_b && _w && !_sensor) {
		if(!_b->applyRicochet(cpArbiterGetNormal(arb, 0)))
			_b->onWallCollide(_w);
	}

    return 1;
}

int PhysicsImpl::sensorStart(GObject* radarAgent, GObject* target, cpArbiter* arb)
{
    RadarObject* radarObject = dynamic_cast<RadarObject*>(radarAgent);

	if (radarObject) {
        if(logPhysicsHandlers)
            log("%s sensed %s.", radarObject->name.c_str(), target->name.c_str());
		radarObject->radarCollision(target);
	}
	else {
		log("sensorStart: %s is not a radar object", radarAgent->name.c_str());
	}

	return 1;
}

void PhysicsImpl::sensorEnd(GObject* radarAgent, GObject* target, cpArbiter* arb)
{
    RadarObject* radarObject = dynamic_cast<RadarObject*>(radarAgent);
    
	if (radarObject) {
        if(logPhysicsHandlers)
            log("%s lost %s.", radarObject->name.c_str(), target->name.c_str());
		radarObject->radarEndCollision(target);
	}
	else {
		log("sensorEnd: %s is not a radar object", radarAgent->name.c_str());
	}
}

int PhysicsImpl::floorObjectBegin(GObject* floorSegment, GObject* obj, cpArbiter* arb)
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

void PhysicsImpl::floorObjectEnd(GObject* floorSegment, GObject* obj, cpArbiter* arb)
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

int PhysicsImpl::playerAreaSensorBegin(GObject* a, GObject *b, cpArbiter* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerContact(p);
	}
	return 1;
}

void PhysicsImpl::playerAreaSensorEnd(GObject* a, GObject *b, cpArbiter* arb)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerEndContact(p);
	}
}

int PhysicsImpl::enemyAreaSensorBegin(GObject* a, GObject *b, cpArbiter* arb)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyContact(e);
	}
	return 1;
}

void PhysicsImpl::enemyAreaSensorEnd(GObject* a, GObject *b, cpArbiter* arb)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyEndContact(e);
	}
}

int PhysicsImpl::npcAreaSensorBegin(GObject* a, GObject *b, cpArbiter* arb)
{
	Agent* npc = dynamic_cast<Agent*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (npc && as) {
		as->onNPCContact(npc);
	}
	return 1;
}

void PhysicsImpl::npcAreaSensorEnd(GObject* a, GObject *b, cpArbiter* arb)
{
	Agent* npc = dynamic_cast<Agent*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (npc && as) {
		as->onNPCEndContact(npc);
	}
}

int PhysicsImpl::environmentAreaSensorBegin(GObject* obj, GObject* areaSensor, cpArbiter* arb)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectContact(obj);
	}

	return 1;
}

void PhysicsImpl::environmentAreaSensorEnd(GObject* areaSensor, GObject* obj, cpArbiter* arb)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectEndContact(obj);
	}
}
