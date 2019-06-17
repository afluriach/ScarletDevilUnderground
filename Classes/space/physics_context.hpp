//
//  physics_context.hpp
//  Koumachika
//
//  Created by Toni on 6/8/19.
//
//

#ifndef physics_context_hpp
#define physics_context_hpp

class GSpace;
class InteractibleObject;

class physics_context
{
public:
	static const bool logBodyCreation;

	inline physics_context(GSpace* space) : space(space) {}

	pair<cpShape*, cpBody*> createCircleBody(
		const SpaceVect& center,
		SpaceFloat radius,
		SpaceFloat mass,
		GType type,
		PhysicsLayers layers,
		bool sensor,
		GObject* obj
	);
	pair<cpShape*, cpBody*> createRectangleBody(
		const SpaceVect& center,
		const SpaceVect& dim,
		SpaceFloat mass,
		GType type,
		PhysicsLayers layers,
		bool sensor,
		GObject* obj
	);

	GObject * queryAdjacentTiles(
		SpaceVect pos,
		GType type,
		PhysicsLayers layers,
		type_index t
	) const;

	GObject * pointQuery(
		SpaceVect pos,
		GType type,
		PhysicsLayers layers
	) const;

	bool rectangleQuery(
		SpaceVect center,
		SpaceVect dimensions,
		GType type,
		PhysicsLayers layers,
		SpaceFloat angle = 0.0
	) const;

	SpaceFloat rectangleFeelerQuery(
		const GObject* agent, 
		SpaceVect center,
		SpaceVect dimensions,
		GType type,
		PhysicsLayers layers,
		SpaceFloat angle
	) const;

	unordered_set<GObject*> rectangleObjectQuery(
		SpaceVect center,
		SpaceVect dimensions,
		GType type,
		PhysicsLayers layers,
		SpaceFloat angle = 0.0
	) const;

	bool obstacleRadiusQuery(
		const GObject* agent,
		SpaceVect center,
		SpaceFloat radius,
		GType type,
		PhysicsLayers layers
	) const;

	unordered_set<GObject*> radiusQuery(
		const GObject* agent,
		SpaceVect center,
		SpaceFloat radius,
		GType type,
		PhysicsLayers layers
	) const;

	template<class C>
	inline unordered_set<C*> radiusQueryByType(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers)
	{
		unordered_set<GObject*> objects = radiusQuery(agent, center, radius, type, layers);
		unordered_set<C*> result;

		for (GObject* obj : objects) {
			C* c = dynamic_cast<C*>(obj);
			if (c) result.insert(c);
		}
		return result;
	}

	template<class C>
	unordered_set<C*> rectangleQueryByType(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle = 0.0)
	{
		unordered_set<GObject*> objects = rectangleObjectQuery(center, dimensions, type, layers, angle);
		unordered_set<C*> result;

		for (GObject* obj : objects) {
			C* c = dynamic_cast<C*>(obj);
			if (c) result.insert(c);
		}
		return result;
	}

	SpaceFloat distanceFeeler(const GObject * agent, SpaceVect feeler, GType gtype) const;
	SpaceFloat distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const;

	//uses rectangle query (width should be diameter of agent)
	SpaceFloat obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const;
	bool obstacleToTarget(const GObject * agent, const GObject* target, SpaceFloat width) const;
	//uses line/ray query
	SpaceFloat obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler) const;
	SpaceFloat wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const;
	SpaceFloat trapFloorDistanceFeeler(const GObject* agent, SpaceVect feeler) const;

	bool feeler(const GObject * agent, SpaceVect feeler, GType gtype) const;
	bool feeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const;
	GObject* objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const;
	//uses rectangle query (width should be diameter of agent)
	bool obstacleFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const;
	//uses line/ray
	bool obstacleFeeler(const GObject * agent, SpaceVect feeler) const;
	bool wallFeeler(const GObject * agent, SpaceVect feeler) const;
	InteractibleObject* interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const;

	bool lineOfSight(const GObject * agent, const GObject * target) const;
protected:

	GSpace* space;
};

#endif 
