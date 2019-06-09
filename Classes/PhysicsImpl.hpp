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

#include "types.h"

class PhysicsImpl
{
public:
	typedef pair<GObject*, GObject*> object_pair;
	typedef pair<GType, GType> collision_type;
	typedef pair<object_pair, collision_type> contact;

	static const bool logPhysicsHandlers;

	static int beginContact(cpArbiter* arb, cpSpace* space, void* data);
	static void endContact(cpArbiter* arb, cpSpace* space, void* data);

	PhysicsImpl(GSpace* space);

	void addCollisionHandlers();

protected:

	template<GType TypeA, GType TypeB>
	inline void AddHandler(int(PhysicsImpl::*begin)(GObject*, GObject*, cpArbiter*), void(PhysicsImpl::*end)(GObject*, GObject*, cpArbiter*))
	{
		cpSpaceAddCollisionHandler(
			physicsSpace,
			to_uint(TypeA),
			to_uint(TypeB),
			&PhysicsImpl::beginContact,
			static_cast<cpCollisionPreSolveFunc>(nullptr),
			static_cast<cpCollisionPostSolveFunc>(nullptr),
			&PhysicsImpl::endContact,
			this
		);

		if (begin)
			beginContactHandlers[collision_type(TypeA, TypeB)] = begin;
		if (end)
			endContactHandlers[collision_type(TypeA, TypeB)] = end;
	}

	void logHandler(const string& base, cpArbiter* arb);
	void logHandler(const string& name, GObject* a, GObject* b);

	int playerEnemyBegin(GObject* a, GObject* b, cpArbiter* arb);
	void playerEnemyEnd(GObject* a, GObject* b, cpArbiter* arb);
	int playerEnemyBulletBegin(GObject* playerObj, GObject* bullet, cpArbiter* arb);
	int playerGrazeRadarBegin(GObject* playerObj, GObject* bullet, cpArbiter* arb);
	void playerGrazeRadarEnd(GObject* playerObj, GObject* bullet, cpArbiter* arb);
	int playerBulletEnemyBegin(GObject* a, GObject* b, cpArbiter* arb);
	int bulletBulletBegin(GObject* a, GObject* b, cpArbiter* arb);
	int playerFlowerBegin(GObject* a, GObject* b, cpArbiter* arb);
	int playerPickupBegin(GObject* a, GObject* b, cpArbiter* arb);
	int bulletEnvironment(GObject* a, GObject* b, cpArbiter* arb);
	int noCollide(GObject* a, GObject* b, cpArbiter* arb);
	int collide(GObject* a, GObject* b, cpArbiter* arb);
	int bulletWall(GObject* bullet, GObject* unused, cpArbiter* arb);
	int sensorStart(GObject* radarAgent, GObject* target, cpArbiter* arb);
	void sensorEnd(GObject* radarAgent, GObject* target, cpArbiter* arb);
	int floorObjectBegin(GObject* floorSegment, GObject* obj, cpArbiter* arb);
	void floorObjectEnd(GObject* floorSegment, GObject* obj, cpArbiter* arb);
	int playerAreaSensorBegin(GObject* a, GObject *b, cpArbiter* arb);
	void playerAreaSensorEnd(GObject* a, GObject *b, cpArbiter* arb);
	int enemyAreaSensorBegin(GObject* a, GObject *b, cpArbiter* arb);
	void enemyAreaSensorEnd(GObject* a, GObject *b, cpArbiter* arb);
	int npcAreaSensorBegin(GObject* a, GObject *b, cpArbiter* arb);
	void npcAreaSensorEnd(GObject* a, GObject *b, cpArbiter* arb);
	int environmentAreaSensorBegin(GObject* obj, GObject* areaSensor, cpArbiter* arb);
	void environmentAreaSensorEnd(GObject* obj, GObject* areaSensor, cpArbiter* arb);

	GSpace* gspace;
	cpSpace* physicsSpace;

	unordered_map<collision_type, int(PhysicsImpl::*)(GObject*, GObject*, cpArbiter*), boost::hash<collision_type>> beginContactHandlers;
	unordered_map<collision_type, void(PhysicsImpl::*)(GObject*, GObject*, cpArbiter*), boost::hash<collision_type>> endContactHandlers;
};

#endif 
