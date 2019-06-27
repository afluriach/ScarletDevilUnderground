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

	typedef pair<GObject*, GObject*> object_pair;
	typedef pair<GType, GType> collision_type;
	typedef pair<object_pair, collision_type> contact;

	static const bool logPhysicsHandlers;
	static const int positionSteps;
	static const int velocitySteps;

	static int beginContact(b2Contact* arb, b2World* space, void* data);
	static void endContact(b2Contact* arb, b2World* space, void* data);
	static int beginContactSensor(b2Contact* arb, b2World* space, void* data);
	static void endContactSensor(b2Contact* arb, b2World* space, void* data);

	PhysicsImpl(GSpace* space);

	void addCollisionHandlers();

protected:

	void AddHandler(
		collision_type types,
		int(PhysicsImpl::*begin)(GObject*, GObject*, b2Contact*),
		void(PhysicsImpl::*end)(GObject*, GObject*, b2Contact*)
	);

	void logHandler(const string& base, b2Contact* arb);
	void logHandler(const string& name, GObject* a, GObject* b);

	int playerEnemyBegin(GObject* a, GObject* b, b2Contact* arb);
	void playerEnemyEnd(GObject* a, GObject* b, b2Contact* arb);
	int playerEnemyBulletBegin(GObject* playerObj, GObject* bullet, b2Contact* arb);
	int playerBulletEnemyBegin(GObject* a, GObject* b, b2Contact* arb);
	int bulletBulletBegin(GObject* a, GObject* b, b2Contact* arb);
	int playerFlowerBegin(GObject* a, GObject* b, b2Contact* arb);
	int playerPickupBegin(GObject* a, GObject* b, b2Contact* arb);
	int bulletEnvironment(GObject* a, GObject* b, b2Contact* arb);
	int noCollide(GObject* a, GObject* b, b2Contact* arb);
	int collide(GObject* a, GObject* b, b2Contact* arb);
	int bulletWall(GObject* bullet, GObject* unused, b2Contact* arb);
	int floorObjectBegin(GObject* floorSegment, GObject* obj, b2Contact* arb);
	void floorObjectEnd(GObject* floorSegment, GObject* obj, b2Contact* arb);
	int playerAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb);
	void playerAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb);
	int enemyAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb);
	void enemyAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb);
	int npcAreaSensorBegin(GObject* a, GObject *b, b2Contact* arb);
	void npcAreaSensorEnd(GObject* a, GObject *b, b2Contact* arb);
	int environmentAreaSensorBegin(GObject* obj, GObject* areaSensor, b2Contact* arb);
	void environmentAreaSensorEnd(GObject* obj, GObject* areaSensor, b2Contact* arb);

	int sensorStart(RadarSensor* radar, GObject* target, b2Contact* arb);
	void sensorEnd(RadarSensor* radar, GObject* target, b2Contact* arb);

	GSpace* gspace;
	b2World* physicsSpace;
	unique_ptr<ContactListener> contactListener;

	unordered_map<collision_type, int(PhysicsImpl::*)(GObject*, GObject*, b2Contact*), boost::hash<collision_type>> beginContactHandlers;
	unordered_map<collision_type, void(PhysicsImpl::*)(GObject*, GObject*, b2Contact*), boost::hash<collision_type>> endContactHandlers;
};

#endif 
