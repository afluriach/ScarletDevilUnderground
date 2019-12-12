//
//  PhysicsImpl.hpp
//  Koumachika
//
//  Created by Toni on 6/8/19.
//
//

#ifndef PhysicsImpl_hpp
#define PhysicsImpl_hpp

class GObject;
class GSpace;

class ContactListener : public b2ContactListener
{
public:
	inline ContactListener(PhysicsImpl* phys) : phys(phys) {}

	virtual void BeginContact(b2Contact* contact);
	virtual void EndContact(b2Contact* contact);
	PhysicsImpl* phys;
};

class PhysicsImpl
{
public:
	friend class ContactListener;
	friend class physics_context;

	typedef pair<GObject*, GObject*> object_pair;
	typedef pair<GType, GType> collision_type;
	typedef pair<object_pair, collision_type> contact;
	typedef function<void(b2Contact*)> contact_func;
	typedef void(*pairwise_obj_func)(GObject*, GObject*, b2Contact*);
	typedef void(*radarsensor_func)(RadarSensor*, GObject*, b2Contact*);

	static const bool logPhysicsHandlers;
	static const int positionSteps;
	static const int velocitySteps;

	static const unordered_map<GType, int> typeGroups;

	static int getGroup(GType);

	PhysicsImpl(GSpace* space);

	void addCollisionHandlers();

protected:

	pair<b2Body*, b2Fixture*> createCircleBody(
		const SpaceVect& center,
		SpaceFloat radius,
		SpaceFloat mass,
		GType type,
		PhysicsLayers layers,
		bool sensor,
		void* obj
	);
	pair<b2Body*, b2Fixture*> createRectangleBody(
		const SpaceVect& center,
		const SpaceVect& dim,
		SpaceFloat mass,
		GType type,
		PhysicsLayers layers,
		bool sensor,
		void* obj
	);

	b2Filter generateFilter(GType type, PhysicsLayers layers);
	b2BodyDef generateBodyDef(GType type, SpaceVect center, SpaceFloat mass);
	void checkAddNavObstacle(GType type, SpaceVect center, SpaceVect dim, SpaceFloat mass);

	void addCollide(GType a, GType b);

	//This function will set both types to collide with each other pairwise in 
	//the collision masks. It will also install the callback with the lower
	//numeric value first, except for pairs of the same type.
	void AddHandler(
		collision_type types,
		contact_func begin,
		contact_func end
	);

	GSpace* gspace;
	b2World* world;
	unique_ptr<ContactListener> contactListener;

	unordered_map<GType, unsigned int> collisionMasks;

	unordered_map<
		collision_type,
		contact_func,
		boost::hash<collision_type>
	> beginContactHandlers;
	
	unordered_map<
		collision_type,
		contact_func,
		boost::hash<collision_type>
	> endContactHandlers;
};

#endif 
