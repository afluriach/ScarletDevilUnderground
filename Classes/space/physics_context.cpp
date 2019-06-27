//
//  physics_context.cpp
//  Koumachika
//
//  Created by Toni on 6/8/19.
//
//

#include "Prefix.h"

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "GSpace.hpp"
#include "physics_context.hpp"

const bool physics_context::logBodyCreation = false;

#define g(x) to_uint(GType::x)

const unordered_map<GType, uint32> physics_context::collisionMasks = {
	{ GType::areaSensor, g(player) | g(enemy) | g(npc) | g(environment) },
	{ GType::bomb, g(player) | g(enemy) | g(npc) | g(environment) | g(wall) | g(floorSegment) },
	{ GType::enemy, g(player) | g(playerBullet) | g(playerGrazeRadar) | g(enemy) | g(environment) | g(npc) | g(floorSegment)| g(areaSensor) | g(wall)},
	{ GType::enemyBullet, g(player) | g(playerBullet) | g(playerGrazeRadar) | g(environment) | g(wall) },
	{ GType::enemySensor, g(player) | g(playerBullet) | g(enemy) | g(bomb)},
	{ GType::environment, g(player) | g(playerBullet) | g(enemy) | g(enemyBullet) | g(environment) | g(wall) },
	{ GType::foliage, g(player)},
	{ GType::floorSegment, g(enemy) | g(environment) | g(npc) | g(player) },
	{ GType::npc, g(player) | g(environment) | g(wall)},
	{ GType::player, g(enemy) | g(enemyBullet) | g(environment) | g(playerPickup) | g(npc) | g(floorSegment) | g(areaSensor) | g(wall) | g(enemySensor) },
	{ GType::playerPickup, g(player) },
	{ GType::playerBullet, g(enemy) | g(enemyBullet) | g(environment) | g(npc) | g(wall) },
	{ GType::playerGrazeRadar, g(enemyBullet) },
	{ GType::wall, g(player) | g(playerBullet) | g(enemy) | g(enemyBullet) | g(environment) | g(npc) | g(floorSegment) | g(areaSensor) }
};

b2BodyType getMassType(SpaceFloat mass)
{
	if (mass < 0.0)
		return b2_staticBody;
	else if (mass > 0.0)
		return b2_dynamicBody;
	else
		return b2_kinematicBody;
}

//void setShapeProperties(cpShape* shape, PhysicsLayers layers, GType type, bool sensor)
//{
//	shape->layers = to_uint(layers);
//	shape->group = 0;
//	shape->collision_type = to_uint(type);
//	shape->sensor = sensor;
//}

pair<b2Body*, b2Fixture*> physics_context::createCircleBody(
    const SpaceVect& center,
    SpaceFloat radius,
    SpaceFloat mass,
    GType type,
    PhysicsLayers layers,
    bool sensor,
    void* obj
){    
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

	body = space->world->CreateBody(&def);

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
	fixture.filter.groupIndex = 0;
	fixture.filter.layers = to_uint(layers);

	shape = body->CreateFixture(&fixture);

	if (def.type == b2_staticBody && (type == GType::environment || type == GType::wall)) {
		space->addNavObstacle(center, SpaceVect(radius*2.0, radius*2.0));
	}

    return make_pair(body,shape);
}

pair<b2Body*, b2Fixture*> physics_context::createRectangleBody(
    const SpaceVect& center,
    const SpaceVect& dim,
    SpaceFloat mass,
    GType type,
    PhysicsLayers layers,
    bool sensor,
    void* obj
){
    
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

	body = space->world->CreateBody(&def);

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
	fixture.filter.groupIndex = 0.0;
	fixture.filter.layers = to_uint(layers);

	shape = body->CreateFixture(&fixture);

	if (mass < 0.0 && (type == GType::environment || type == GType::wall)) {
		space->addNavObstacle(center, dim);
	}


	return make_pair(body, shape);
}

//Static bodies are not actually added to the physics engine, but they 
//need to be deallocated.
void physics_context::removeObject(b2Body* body)
{
	space->world->DestroyBody(body);
}

SpaceFloat physics_context::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
	return distanceFeeler(agent, _feeler, gtype, PhysicsLayers::all);
}

SpaceFloat physics_context::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;

	//Distance along the segment is scaled [0,1].
	SpaceFloat closest = 1.0;

	b2RayCastCallback callback = [gtype, layers, &closest](b2Fixture* fixture, const SpaceVect& point, const SpaceVect& normal, float64 fraction)-> float64 {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if ( bitwise_and_bool(type, gtype) && bitwise_and_bool(obj->getLayers(), layers)) {
			closest = fraction;
		}
		return 0.0;
	};

	space->world->RayCast(callback, toBox2D(start), toBox2D(end));

	return closest * _feeler.length();
}

SpaceFloat physics_context::wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const
{
    return distanceFeeler(agent, feeler, GType::wall);
}

SpaceFloat physics_context::obstacleDistanceFeeler(const GObject * agent, SpaceVect _feeler) const
{
	SpaceFloat d = distanceFeeler(
		agent,
		_feeler,
		agentObstacles,
		agent->getCrntLayers()
	);

    return min(d, agent->isOnFloor() ? trapFloorDistanceFeeler(agent,_feeler) : _feeler.length());
}

SpaceFloat physics_context::obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const
{
	SpaceVect start = agent->getPos();
	SpaceVect center = agent->getPos() + feeler*0.5;
	SpaceVect dimensions(feeler.length(), width);

	return rectangleFeelerQuery(
		agent,
		center,
		dimensions,
		agentObstacles,
		agent->getCrntLayers(),
		feeler.toAngle()
	);
}

bool physics_context::obstacleToTarget(const GObject * agent, const GObject* target, SpaceFloat width) const
{
	SpaceVect start = agent->getPos();
	SpaceVect endpoint = target->getPos();
	SpaceVect feeler = endpoint - start;

	SpaceVect center = start + feeler * 0.5;
	SpaceVect dimensions(feeler.length(), width);

	unordered_set<GObject*> result = rectangleObjectQuery(
		center,
		dimensions,
		agentObstacles,
		agent->getCrntLayers(), feeler.toAngle()
	);
	
	if (result.size() > 2)
		return true;

	for (GObject* obj : result) {
		if (obj != agent && obj != target)
			return true;
	}

	return false;
}

SpaceFloat physics_context::trapFloorDistanceFeeler(const GObject* agent, SpaceVect feeler) const
{
	return distanceFeeler(agent, feeler, GType::floorSegment, PhysicsLayers::belowFloor);
}

bool physics_context::feeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
    return feeler(agent,_feeler, gtype, PhysicsLayers::all);
}

bool physics_context::feeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
	bool collision = false;

	b2RayCastCallback callback = [gtype, layers, &collision](b2Fixture* fixture, const SpaceVect& point, const SpaceVect& normal, float64 fraction)-> float64 {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if ( bitwise_and_bool(type,gtype) && bitwise_and_bool(obj->getLayers(), layers)) {
			collision = true;
		}
		return 0.0;
	};

	space->world->RayCast(callback, toBox2D(start), toBox2D(end));

	return collision;
}

GObject* physics_context::objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const
{
	SpaceVect start = agent->getPos();
	SpaceVect end = start + feeler;
	GObject* bestResult = nullptr;
	SpaceFloat bestRatio = 1.0;

	b2RayCastCallback callback = [gtype, layers, &bestRatio, &bestResult](b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float64 fraction)-> float64 {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if (
			obj &&
			fraction < bestRatio &&
			bitwise_and_bool(type, gtype) &&
			bitwise_and_bool(obj->getLayers(), layers)
		) {
			bestResult = obj;
			bestRatio = fraction;
		}
		return fraction;
	};

	space->world->RayCast(callback, toBox2D(start), toBox2D(end));

	return bestResult;
}

bool physics_context::wallFeeler(const GObject * agent, SpaceVect _feeler) const
{
    return feeler(agent, _feeler, GType::wall);
}

bool physics_context::obstacleFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const
{
	return obstacleDistanceFeeler(agent, feeler, width) < feeler.length();
}

bool physics_context::obstacleFeeler(const GObject * agent, SpaceVect _feeler) const
{
	return feeler(
		agent,
		_feeler,
		agentObstacles,
		agent->getCrntLayers()
	);
}

GObject* physics_context::interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const
{
	log("interactible: %f,%f", feeler.length(), feeler.toAngle());

	GObject* obj = objectFeeler(
		agent,
		feeler,
		interactibleObjects,
		agent->getCrntLayers()
	);

	return obj;
}

bool physics_context::lineOfSight(const GObject* agent, const GObject * target) const
{
    SpaceVect feeler_displacement = target->getPos() - agent->getPos();

	return !feeler(
		agent,
		feeler_displacement,
		enum_bitwise_or(GType,environment, wall),
		PhysicsLayers::eyeLevel
	);
}

GObject * physics_context::queryAdjacentTiles(
	SpaceVect pos,
	GType type,
	PhysicsLayers layers,
	type_index t
) const
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

GObject * physics_context::pointQuery(SpaceVect pos, GType type, PhysicsLayers layers) const
{
	GObject* result = nullptr;

	b2QueryCallback callback = [type, layers, &result](b2Fixture* fixture) -> bool {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType _type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if ( bitwise_and_bool(type,_type) && bitwise_and_bool(obj->getLayers(), layers)) {
			result = obj;
			return false;
		}
		return true;
	};

	space->world->QueryAABB(callback, b2AABB{ toBox2D(pos), toBox2D(pos) });

	return result;
}

bool physics_context::rectangleQuery(
	SpaceVect center,
	SpaceVect dimensions,
	GType type,
	PhysicsLayers layers,
	SpaceFloat angle
) const
{
	bool collision = false;

	b2QueryCallback callback = [type, layers, &collision](b2Fixture* fixture) -> bool {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType _type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if (bitwise_and_bool(type, _type) && bitwise_and_bool(obj->getLayers(), layers)) {
			collision = true;
			return false;
		}
		return true;
	};

	space->world->QueryAABB(
		callback,
		b2AABB{ toBox2D(center - 0.5*dimensions), toBox2D(center + 0.5 * dimensions) }
	);

	return collision;
}

SpaceFloat physics_context::rectangleFeelerQuery(
	const GObject* agent,
	SpaceVect center,
	SpaceVect dimensions,
	GType type,
	PhysicsLayers layers,
	SpaceFloat angle
) const
{
	SpaceVect _feeler = SpaceVect::ray(dimensions.x, angle);

	return distanceFeeler(agent, _feeler, type, layers);
}

unordered_set<GObject*> physics_context::rectangleObjectQuery(
	SpaceVect center,
	SpaceVect dimensions,
	GType type,
	PhysicsLayers layers,
	SpaceFloat angle
) const
{
	unordered_set<GObject*> result;

	b2QueryCallback callback = [type, layers, &result](b2Fixture* fixture) -> bool {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType _type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if (bitwise_and_bool(type, _type) && bitwise_and_bool(obj->getLayers(), layers) && obj) {
			result.insert(obj);
		}
		return true;
	};

	space->world->QueryAABB(
		callback,
		b2AABB{ toBox2D(center - 0.5*dimensions), toBox2D(center + 0.5 * dimensions) }
	);

	return result;
}

bool physics_context::obstacleRadiusQuery(
	const GObject* agent,
	SpaceVect center,
	SpaceFloat radius,
	GType type,
	PhysicsLayers layers
) const
{
	return radiusQuery(agent, center, radius, type, layers).size() > 0;
}

unordered_set<GObject*> physics_context::radiusQuery(
	const GObject* agent,
	SpaceVect center,
	SpaceFloat radius,
	GType type,
	PhysicsLayers layers
) const
{
	unordered_set<GObject*> result;
	SpaceVect rv(radius, radius);
	SpaceFloat agentRadius = agent ? agent->getRadius() : 0.0;

	b2QueryCallback callback = [type, layers, center, radius, agentRadius, &result](b2Fixture* fixture) -> bool {
		GObject* obj = static_cast<GObject*>(fixture->GetUserData());
		GType _type = static_cast<GType>(fixture->GetFilterData().categoryBits);
		if (bitwise_and_bool(type, _type) && bitwise_and_bool(obj->getLayers(), layers) && obj) {
			SpaceFloat dist = (center - obj->getPos()).length();

			if(dist < radius - agentRadius - obj->getRadius())
				result.insert(obj);
		}
		return true;
	};

	space->world->QueryAABB(
		callback,
		b2AABB{ toBox2D(center - rv), toBox2D(center + rv) }
	);

	return result;
}
