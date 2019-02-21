//
//  GSpace_physics.cpp
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
#include "FloorSegment.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "Player.hpp"
#include "Upgrade.hpp"
#include "util.h"
#include "Wall.hpp"

//BEGIN PHYSICS
#define _addHandler(a,b,begin,end) AddHandler<GType::a, GType::b>(&GSpace::begin,&GSpace::end)
#define _addHandlerNoEnd(a,b,begin) AddHandler<GType::a, GType::b>(&GSpace::begin,nullptr)

int GSpace::beginContact(cpArbiter* arb, cpSpace* space, void* data)
{
	OBJS_FROM_ARB

		GSpace* _this = static_cast<GSpace*>(data);

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
		int(GSpace::*begin_method)(GObject*, GObject*) = it->second;
		(_this->*begin_method)(a, b);
	}

	return 1;
}

void GSpace::endContact(cpArbiter* arb, cpSpace* space, void* data)
{
	OBJS_FROM_ARB

		GSpace* _this = static_cast<GSpace*>(data);

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
		void(GSpace::*end_method)(GObject*, GObject*) = it->second;
		(_this->*end_method)(a, b);
	}
}


void GSpace::addCollisionHandlers()
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
    _addHandlerNoEnd(player,collectible,playerCollectibleBegin);
	_addHandlerNoEnd(player, upgrade, playerUpgradeBegin);
    _addHandlerNoEnd(player,npc,collide);
	_addHandlerNoEnd(playerBullet, wall, bulletWall);
	_addHandlerNoEnd(enemyBullet, wall, bulletWall);    

	_addHandlerNoEnd(enemy, enemy, collide);
	_addHandlerNoEnd(environment, environment, collide);
	_addHandlerNoEnd(npc, npc, collide);

	_addHandler(enemySensor, player, sensorStart, sensorEnd);
	_addHandler(enemySensor, playerBullet, sensorStart, sensorEnd);
	_addHandler(enemySensor, enemy, sensorStart, sensorEnd);

	_addHandler(floorSegment, player, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, enemy, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, npc, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, collectible, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, environment, floorObjectBegin, floorObjectEnd);

	_addHandler(player, areaSensor, playerAreaSensorBegin, playerAreaSensorEnd);
	_addHandler(enemy, areaSensor, enemyAreaSensorBegin, enemyAreaSensorEnd);
	_addHandler(environment, areaSensor, environmentAreaSensorBegin, environmentAreaSensorEnd);
}

const bool GSpace::logBodyCreation = false;
const bool GSpace::logPhysicsHandlers = false;

void setShapeProperties(cpShape* shape, PhysicsLayers layers, GType type, bool sensor)
{
    shape->layers = to_uint(layers);
    shape->group = 0;
	shape->collision_type = to_uint(type);
	shape->sensor = sensor;
}

pair<cpShape*, cpBody*> GSpace::createCircleBody(
    const SpaceVect& center,
    SpaceFloat radius,
    SpaceFloat mass,
    GType type,
    PhysicsLayers layers,
    bool sensor,
    GObject* obj)
{
    if(logBodyCreation) log(
        "createCircleBody for %s at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(center),
        mass
    );
    
    if(radius == 0)
        log("createCircleBody: zero radius for %s.", obj->name.c_str());
    
	cpBody* body;
	cpShape* shape;

    if(mass <= 0.0){
        body = cpBodyNewStatic();
        if(type == GType::environment || type == GType::wall)
            addNavObstacle(center, SpaceVect(radius*2.0, radius*2.0));
    }
    else{
        body = cpBodyNew(mass, circleMomentOfInertia(mass, radius));
		cpSpaceAddBody(space, body);
	}

	cpBodySetPos(body, center);

    shape = cpCircleShapeNew(body, radius, cpvzero);
	cpSpaceAddShape(space, shape);
    
    setShapeProperties(shape, layers, type, sensor);
    
	shape->data = obj;
	body->data = obj;

    return make_pair(shape,body);
}

pair<cpShape*, cpBody*> GSpace::createRectangleBody(
    const SpaceVect& center,
    const SpaceVect& dim,
    SpaceFloat mass,
    GType type,
    PhysicsLayers layers,
    bool sensor,
    GObject* obj)
{
    if(logBodyCreation && obj) log(
        "Creating rectangle body for %s. %f x %f at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(dim),
        expand_vector2(center),
        mass
    );
    
    if(dim.x == 0 && obj)
        log("createRectangleBody: zero width for %s.", obj->name.c_str());
    if(dim.y == 0 && obj)
        log("createRectangleBody: zero height for %s.", obj->name.c_str());

	cpBody* body;
	cpShape* shape;

	if (mass <= 0.0) {
		body = cpBodyNewStatic();
		if (type == GType::environment || type == GType::wall)
			addNavObstacle(center, dim);
	}
	else {
		body = cpBodyNew(mass, rectangleMomentOfInertia(mass, dim));
		cpSpaceAddBody(space, body);
	}
    
	cpBodySetPos(body, center);

	shape = cpBoxShapeNew(body, dim.x, dim.y);
	cpSpaceAddShape(space, shape);

	setShapeProperties(shape, layers, type, sensor);

	shape->data = obj;
	body->data = obj;

	return make_pair(shape, body);
}

void GSpace::logHandler(const string& base, cpArbiter* arb)
{
    if(logPhysicsHandlers){
        OBJS_FROM_ARB
        
        log("%s: %s, %s", base.c_str(), a->name.c_str(), b->name.c_str());
    }
}

void GSpace::logHandler(const string& name, GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s: %s, %s", name.c_str(), a->name.c_str(), b->name.c_str());
}

int GSpace::playerEnemyBegin(GObject* a, GObject* b)
{    
    Player* p = dynamic_cast<Player*>(a);
    Enemy* e = dynamic_cast<Enemy*>(b);
    
    if(!p)
        log("%s is not a Player", a->getName().c_str());
    if(!e)
        log("%s is not an Enemy", b->getName().c_str());
    
    if(p && e)
        e->onTouchPlayer(p);

    return 1;
}

void GSpace::playerEnemyEnd(GObject* a, GObject* b)
{
	Enemy* e = dynamic_cast<Enemy*>(b);

	if(e)
		e->endTouchPlayer();
    
	logHandler("playerEnemyEnd", a,b);
}

int GSpace::playerEnemyBulletBegin(GObject* playerObj, GObject* bullet)
{
    Player* player = dynamic_cast<Player*>(playerObj);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

    if(logPhysicsHandlers)
        log("%s hit by %s", player->name.c_str(), bullet->name.c_str());

	if (player && _bullet) {
		_bullet->onAgentCollide(player);
		player->onBulletCollide(_bullet);
		_bullet->invalidateGraze();
	}

    return 1;
}

int GSpace::playerGrazeRadarBegin(GObject* playerRadar, GObject* bullet)
{
	Player* player = dynamic_cast<Player*>(playerRadar);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

	if (player && _bullet) {
		player->onGrazeTouch(_bullet);
	}

	return 1;
}

void GSpace::playerGrazeRadarEnd(GObject* playerRadar, GObject* bullet)
{
	Player* player = dynamic_cast<Player*>(playerRadar);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

	if (player && _bullet) {
		player->onGrazeCleared(_bullet);
	}
}

int GSpace::playerBulletEnemyBegin(GObject* a, GObject* b)
{    
    Bullet* bullet = dynamic_cast<Bullet*>(a);
    Agent* _enemy_agent = dynamic_cast<Agent*>(b);
    
    if(!bullet)
        log("%s is not a Bullet", a->getName().c_str());
    if(!_enemy_agent)
        log("%s is not an Enemy", b->getName().c_str());
    
	if (bullet && _enemy_agent){
		bullet->onAgentCollide(_enemy_agent);
		_enemy_agent->onBulletCollide(bullet);
	}

    if(logPhysicsHandlers)
        log("%s hit by %s", b->name.c_str(), a->name.c_str());
    
    return 1;
}

int GSpace::bulletBulletBegin(GObject* a, GObject* b)
{
	Bullet* _a = dynamic_cast<Bullet*>(a);
	Bullet* _b = dynamic_cast<Bullet*>(b);

	if (_a && _b) {
		_a->onBulletCollide(_b);
		_b->onBulletCollide(_a);
	}

	return 0;
}

int GSpace::playerFlowerBegin(GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s stepped on", b->name.c_str());
    
    return 1;
}

int GSpace::playerCollectibleBegin(GObject* a, GObject* b)
{
    Player* p = dynamic_cast<Player*>(a);
    Collectible* c = dynamic_cast<Collectible*>(b);
    
    if(p && c){
        p->onCollectible(c);
    }
    
    return 0;
}

int GSpace::playerUpgradeBegin(GObject* a, GObject* b)
{
	Player* p = dynamic_cast<Player*>(a);
	Upgrade* u = dynamic_cast<Upgrade*>(b);

	if (p && u) {
		p->applyUpgrade(u);
	}

	return 0;
}


int GSpace::bulletEnvironment(GObject* bullet, GObject* environment)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);

	if (_b && environment) {
		_b->onEnvironmentCollide(environment);
	}
    
    return 1;
}

int GSpace::noCollide(GObject* a, GObject* b)
{
    return 0;
}

int GSpace::collide(GObject* a, GObject* b)
{
    return 1;
}

int GSpace::bulletWall(GObject* bullet, GObject* wall)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	Wall* _w = dynamic_cast<Wall*>(wall);
	bool _sensor = cpShapeGetSensor(wall->bodyShape);

	if (_b && _w && !_sensor) {
		_b->onWallCollide(_w);
	}

    return 1;
}

int GSpace::sensorStart(GObject* radarAgent, GObject* target)
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

void GSpace::sensorEnd(GObject* radarAgent, GObject* target)
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

int GSpace::floorObjectBegin(GObject* floorSegment, GObject* obj)
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
		obj->message<GObject>(obj, &GObject::onContactFloorSegment, object_ref<FloorSegment>(fs));
		return 1;
	}
}

void GSpace::floorObjectEnd(GObject* floorSegment, GObject* obj)
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
		obj->message<GObject>(obj, &GObject::onEndContactFloorSegment, object_ref<FloorSegment>(fs));
	}
}

int GSpace::playerAreaSensorBegin(GObject* a, GObject *b)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerContact(p);
	}
	return 1;
}

void GSpace::playerAreaSensorEnd(GObject* a, GObject *b)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerEndContact(p);
	}
}

int GSpace::enemyAreaSensorBegin(GObject* a, GObject *b)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyContact(e);
	}
	return 1;
}

void GSpace::enemyAreaSensorEnd(GObject* a, GObject *b)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyEndContact(e);
	}
}

int GSpace::environmentAreaSensorBegin(GObject* obj, GObject* areaSensor)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectContact(obj);
	}

	return 1;
}

void GSpace::environmentAreaSensorEnd(GObject* areaSensor, GObject* obj)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectEndContact(obj);
	}
}

//END PHYSICS

//BEGIN SENSORS

const GType GSpace::interactibleObjects = enum_bitwise_or(GType, npc, environment);
const GType GSpace::obstacles = enum_bitwise_or5(GType, wall, enemy, environment, npc, player);

struct FeelerData
{
	//input data
	const GObject* agent = nullptr;
	unsigned int gtype;

	//output data
	double distance = 1.0;
	GObject* result = nullptr;
};

void feelerCallback(cpShape *shape, cpFloat t, cpVect n, void *data)
{
	FeelerData* queryData = static_cast<FeelerData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && (to_uint(shape->collision_type) & queryData->gtype) && obj != queryData->agent && t < queryData->distance && t != 0.0) {
		queryData->distance = t;
		queryData->result = obj;
	}
}

struct PointQueryData
{
	//input data
	const GObject* agent;
	unsigned int gtype;

	//output
	GObject* result = nullptr;
};

void pointQueryCallback(cpShape *shape, void *data)
{
	PointQueryData* queryData = static_cast<PointQueryData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && (to_uint(shape->collision_type) & queryData->gtype)) {
		queryData->result = obj;
	}
}

struct ShapeQueryData
{
	const GObject* agent;
	unsigned gtype;

	set<GObject*> results;
};

void shapeQueryCallback(cpShape *shape, cpContactPointSet *points, void *data)
{
	ShapeQueryData* queryData = static_cast<ShapeQueryData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && obj != queryData->agent && (to_uint(shape->collision_type) & queryData->gtype)) {
		queryData->results.insert(obj);
	}
}

struct FeelerQueryData
{
	const GObject* agent;
	unsigned int gtype;
	cpBody* queryBody;

	SpaceFloat distance = 0.0;
};

void feelerQueryCallback(cpShape* shape, cpContactPointSet* points, void* data)
{
	FeelerQueryData* queryData = static_cast<FeelerQueryData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && obj != queryData->agent && (to_uint(shape->collision_type) & queryData->gtype)) {
		for_irange(i, 0, points->count) {
			SpaceVect local = cpBodyWorld2Local(queryData->queryBody, points->points[i].point);
			queryData->distance = min(queryData->distance, local.x);
		}
	}
}

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
	return distanceFeeler(agent, _feeler, gtype, PhysicsLayers::all);
}

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space, start, end, to_uint(layers), 0, feelerCallback, &queryData);
        
    return queryData.distance*_feeler.length();
}

SpaceFloat GSpace::wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const
{
    return distanceFeeler(agent, feeler, GType::wall);
}

SpaceFloat GSpace::obstacleDistanceFeeler(const GObject * agent, SpaceVect _feeler) const
{
	SpaceFloat d = distanceFeeler(
		agent,
		_feeler,
		obstacles,
		agent->getCrntLayers()
	);

    return min(d, agent->isOnFloor() ? trapFloorDistanceFeeler(agent,_feeler) : _feeler.length());
}

SpaceFloat GSpace::obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const
{
	SpaceVect start = agent->getPos();
	SpaceVect center = agent->getPos() + feeler*0.5;
	SpaceVect dimensions(feeler.length(), width);

	return rectangleFeelerQuery(
		agent,
		center,
		dimensions,
		obstacles,
		PhysicsLayers::all,
		feeler.toAngle()
	);
}

SpaceFloat GSpace::trapFloorDistanceFeeler(const GObject* agent, SpaceVect feeler) const
{
	return distanceFeeler(agent, feeler, GType::floorSegment, PhysicsLayers::belowFloor);
}

bool GSpace::feeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
    return feeler(agent,_feeler, gtype, PhysicsLayers::all);
}

bool GSpace::feeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space, start, end, to_uint(layers), 0, feelerCallback, &queryData);

    return queryData.distance < 1.0;
}

GObject* GSpace::objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const
{
	SpaceVect start = agent->getPos();
	SpaceVect end = start + feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space, start, end, to_uint(layers), 0, feelerCallback, &queryData);

	return queryData.result;
}

bool GSpace::wallFeeler(const GObject * agent, SpaceVect _feeler) const
{
    return feeler(agent, _feeler, GType::wall);
}

bool GSpace::obstacleFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const
{
	return obstacleDistanceFeeler(agent, feeler, width) < feeler.length();
}

bool GSpace::obstacleFeeler(const GObject * agent, SpaceVect _feeler) const
{
	return feeler(
		agent,
		_feeler,
		obstacles,
		agent->getCrntLayers()
	);
}

InteractibleObject* GSpace::interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const
{
	GObject* obj = objectFeeler(
		agent,
		feeler,
		interactibleObjects,
		agent->getCrntLayers()
	);

	return dynamic_cast<InteractibleObject*>(obj);
}

bool GSpace::lineOfSight(const GObject* agent, const GObject * target) const
{
    SpaceVect feeler_displacement = target->getPos() - agent->getPos();

	return !feeler(
		agent,
		feeler_displacement,
		enum_bitwise_or(GType,environment, wall),
		PhysicsLayers::eyeLevel
	);
}

GObject * GSpace::queryAdjacentTiles(SpaceVect pos, GType type, PhysicsLayers layers, type_index t)
{
	enum_foreach(Direction, d, right, end)
	{
		GObject* result = pointQuery(pos + dirToVector(d), type, layers);

		if (result && type_index(typeid(*result)) == t) {
			return result;
		}
	}

	return nullptr;
}

GObject * GSpace::pointQuery(SpaceVect pos, GType type, PhysicsLayers layers)
{
	PointQueryData queryData = { nullptr, to_uint(type) };

	cpSpacePointQuery(space, pos, to_uint(layers), 0, pointQueryCallback, &queryData);

	return queryData.result;
}

bool GSpace::rectangleQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle)
{
	ShapeQueryData data = { nullptr, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space, area, shapeQueryCallback, &data);
	cpBodyFree(body);

	return !data.results.empty();
}

SpaceFloat GSpace::rectangleFeelerQuery(const GObject* agent, SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle) const
{
	cpBody* body = cpBodyNewStatic();
	FeelerQueryData data = { agent, to_uint(type), body, dimensions.x };
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space, area, feelerQueryCallback, &data);
	cpBodyFree(body);

	return data.distance;
}

set<GObject*> GSpace::rectangleObjectQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle)
{
	ShapeQueryData data = { nullptr, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space, area, shapeQueryCallback, &data);
	cpBodyFree(body);

	return data.results;
}

bool GSpace::obstacleRadiusQuery(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers)
{
	return radiusQuery(agent, center, radius, type, layers).size() > 0;
}

set<GObject*> GSpace::radiusQuery(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers)
{
	ShapeQueryData data = { agent, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* circle = cpCircleShapeNew(body, radius, SpaceVect::zero);

	cpBodySetPos(body, center);
	setShapeProperties(circle, layers, GType::none, false);

	cpSpaceShapeQuery(space, circle, shapeQueryCallback, &data);
	cpBodyFree(body);

	return data.results;
}

//END SENSORS
