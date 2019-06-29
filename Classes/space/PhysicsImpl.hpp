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

	void addCollide(GType a, GType b);

	void AddHandler(
		collision_type types,
		void (*begin)(GObject*, GObject*, b2Contact*),
		void(*end)(GObject*, GObject*, b2Contact*)
	);

	GSpace* gspace;
	b2World* world;
	unique_ptr<ContactListener> contactListener;

	unordered_map<GType, unsigned int> collisionMasks;

	unordered_map<
		collision_type,
		pair<void(*)(GObject*, GObject*, b2Contact*), bool>,
		boost::hash<collision_type>
	> beginContactHandlers;
	
	unordered_map<
		collision_type,
		pair<void(*)(GObject*, GObject*, b2Contact*), bool>,
		boost::hash<collision_type>
	> endContactHandlers;
};

#endif 
