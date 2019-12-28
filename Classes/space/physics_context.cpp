//
//  physics_context.cpp
//  Koumachika
//
//  Created by Toni on 6/8/19.
//
//

#include "Prefix.h"

#include "GObject.hpp"
#include "physics_context.hpp"
#include "PhysicsImpl.hpp"

pair<b2Body*, b2Fixture*> physics_context::createCircleBody(
    const SpaceVect& center,
    SpaceFloat radius,
    SpaceFloat mass,
    GType type,
    PhysicsLayers layers,
    bool sensor,
    std::any data
){    
	return space->physicsImpl->createCircleBody(
		center,
		radius,
		mass,
		type,
		layers,
		sensor,
		data
	);

}

pair<b2Body*, b2Fixture*> physics_context::createRectangleBody(
    const SpaceVect& center,
    const SpaceVect& dim,
    SpaceFloat mass,
    GType type,
    PhysicsLayers layers,
    bool sensor,
	std::any data
){
	return space->physicsImpl->createRectangleBody(
		center,
		dim,
		mass,
		type,
		layers,
		sensor,
		data
	);
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

	b2Filter filter;
	filter.maskBits = to_uint(gtype);
	filter.layers = to_uint(layers);

	b2RayCastCallback callback = [&closest](b2Fixture* fixture, const SpaceVect& point, const SpaceVect& normal, float64 fraction)-> float64 {
		closest = fraction;
		return closest;
	};

	space->world->RayCast(callback, toBox2D(start), toBox2D(end), filter);

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

	b2Filter filter;
	filter.maskBits = to_uint(gtype);
	filter.layers = to_uint(layers);

	bool collision = false;

	b2RayCastCallback callback = [&collision](b2Fixture* fixture, const SpaceVect& point, const SpaceVect& normal, float64 fraction)-> float64 {
		collision = true;
		return 0.0;
	};

	space->world->RayCast(callback, toBox2D(start), toBox2D(end), filter);

	return collision;
}

GObject* physics_context::objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const
{
	SpaceVect start = agent->getPos();
	SpaceVect end = start + feeler;
	GObject* bestResult = nullptr;
	SpaceFloat bestRatio = 1.0;

	b2Filter filter;
	filter.maskBits = to_uint(gtype);
	filter.layers = to_uint(layers);

	b2RayCastCallback callback = [&bestRatio, &bestResult](b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float64 fraction)-> float64 {
		GObject* obj = any_cast<GObject*>(fixture->GetUserData());
		if (obj && fraction < bestRatio) {
			bestResult = obj;
			bestRatio = fraction;
			return fraction;
		}
		else {
			return 1.0;
		}
	};

	space->world->RayCast(callback, toBox2D(start), toBox2D(end), filter);

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
	GObject* obj = objectFeeler(
		agent,
		feeler,
		GType::interactible,
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

	b2Filter filter;
	filter.maskBits = to_uint(type);
	filter.layers = to_uint(layers);

	b2QueryCallback callback = [type, layers, &result](b2Fixture* fixture) -> bool {
		GObject* obj = any_cast<GObject*>(fixture->GetUserData());
		if (obj) {
			result = obj;
			return false;
		}
		return true;
	};

	space->world->QueryPoint(callback, toBox2D(pos), filter);

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
	b2PolygonShape rect;
	rect.SetAsBox(dimensions.x * 0.5, dimensions.y * 0.5);
	b2Transform xf;
	xf.Set(toBox2D(center), angle);
	b2Filter filter;
	filter.maskBits = to_uint(type);
	filter.layers = to_uint(layers);

	b2QueryCallback callback = [type, layers, &collision](b2Fixture* fixture) -> bool {
		collision = true;
		return false;
	};

	space->world->QueryShape(callback, xf, &rect, filter);

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
	b2PolygonShape rect;
	rect.SetAsBox(dimensions.x * 0.5, dimensions.y * 0.5);
	b2Transform xf;
	xf.Set(toBox2D(center), angle);
	b2Filter filter;
	filter.maskBits = to_uint(type);
	filter.layers = to_uint(layers);

	b2QueryCallback callback = [type, layers, &result](b2Fixture* fixture) -> bool {
		GObject* obj = any_cast<GObject*>(fixture->GetUserData());
		if (obj) {
			result.insert(obj);
		}
		return true;
	};

	space->world->QueryShape(callback, xf, &rect, filter);

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
	b2CircleShape circle;
	circle.m_radius = radius;
	b2Transform xf;
	xf.Set(toBox2D(center), 0.0);
	b2Filter filter;
	filter.maskBits = to_uint(type);
	filter.layers = to_uint(layers);

	b2QueryCallback callback = [agent, type, layers, center, radius, &result](b2Fixture* fixture) -> bool {
		GObject* obj = any_cast<GObject*>(fixture->GetUserData());
		if (obj && obj != agent) {
			result.insert(obj);
		}
		return true;
	};

	space->world->QueryShape(callback, xf, &circle, filter);

	return result;
}
