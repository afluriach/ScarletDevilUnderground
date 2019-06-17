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

void setShapeProperties(cpShape* shape, PhysicsLayers layers, GType type, bool sensor)
{
	shape->layers = to_uint(layers);
	shape->group = 0;
	shape->collision_type = to_uint(type);
	shape->sensor = sensor;
}

pair<cpShape*, cpBody*> physics_context::createCircleBody(
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
            space->addNavObstacle(center, SpaceVect(radius*2.0, radius*2.0));
    }
    else{
        body = cpBodyNew(mass, circleMomentOfInertia(mass, radius));
		cpSpaceAddBody(space->space, body);
	}

	cpBodySetPos(body, center);

    shape = cpCircleShapeNew(body, radius, cpvzero);
	cpSpaceAddShape(space->space, shape);
    
    setShapeProperties(shape, layers, type, sensor);
    
	shape->data = obj;
	body->data = obj;

    return make_pair(shape,body);
}

pair<cpShape*, cpBody*> physics_context::createRectangleBody(
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
			space->addNavObstacle(center, dim);
	}
	else {
		body = cpBodyNew(mass, rectangleMomentOfInertia(mass, dim));
		cpSpaceAddBody(space->space, body);
	}
    
	cpBodySetPos(body, center);

	shape = cpBoxShapeNew(body, dim.x, dim.y);
	cpSpaceAddShape(space->space, shape);

	setShapeProperties(shape, layers, type, sensor);

	shape->data = obj;
	body->data = obj;

	return make_pair(shape, body);
}

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

	unordered_set<GObject*> results;
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

SpaceFloat physics_context::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
	return distanceFeeler(agent, _feeler, gtype, PhysicsLayers::all);
}

SpaceFloat physics_context::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space->space, start, end, to_uint(layers), 0, feelerCallback, &queryData);
        
    return queryData.distance*_feeler.length();
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
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space->space, start, end, to_uint(layers), 0, feelerCallback, &queryData);

    return queryData.distance < 1.0;
}

GObject* physics_context::objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const
{
	SpaceVect start = agent->getPos();
	SpaceVect end = start + feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space->space, start, end, to_uint(layers), 0, feelerCallback, &queryData);

	return queryData.result;
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

InteractibleObject* physics_context::interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const
{
	GObject* obj = objectFeeler(
		agent,
		feeler,
		interactibleObjects,
		agent->getCrntLayers()
	);

	return dynamic_cast<InteractibleObject*>(obj);
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
	PointQueryData queryData = { nullptr, to_uint(type) };

	cpSpacePointQuery(space->space, pos, to_uint(layers), 0, pointQueryCallback, &queryData);

	return queryData.result;
}

bool physics_context::rectangleQuery(
	SpaceVect center,
	SpaceVect dimensions,
	GType type,
	PhysicsLayers layers,
	SpaceFloat angle
) const
{
	ShapeQueryData data = { nullptr, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space->space, area, shapeQueryCallback, &data);
	cpBodyFree(body);

	return !data.results.empty();
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
	cpBody* body = cpBodyNewStatic();
	FeelerQueryData data = { agent, to_uint(type), body, dimensions.x };
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space->space, area, feelerQueryCallback, &data);
	cpBodyFree(body);

	return data.distance;
}

unordered_set<GObject*> physics_context::rectangleObjectQuery(
	SpaceVect center,
	SpaceVect dimensions,
	GType type,
	PhysicsLayers layers,
	SpaceFloat angle
) const
{
	ShapeQueryData data = { nullptr, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space->space, area, shapeQueryCallback, &data);
	cpBodyFree(body);

	return data.results;
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
	ShapeQueryData data = { agent, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* circle = cpCircleShapeNew(body, radius, SpaceVect::zero);

	cpBodySetPos(body, center);
	setShapeProperties(circle, layers, GType::none, false);

	cpSpaceShapeQuery(space->space, circle, shapeQueryCallback, &data);
	cpBodyFree(body);

	return data.results;
}
