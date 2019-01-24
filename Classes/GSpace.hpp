//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include "Graphics.h"
#include "macros.h"
#include "object_ref.hpp"
#include "scenes.h"
#include "types.h"

class FloorSegment;
class GObject;
class GScene;
class InteractibleObject;

#define OBJS_FROM_ARB \
    GObject* a = static_cast<GObject*>(arb->body_a_private->data); \
    GObject* b = static_cast<GObject*>(arb->body_b_private->data);

template<class D>
constexpr bool isObjectCls() {
	return is_base_of<GObject, D>();
}

#define assert_gobject(cls) static_assert( isObjectCls<cls>(),"Not a GObject type!");

class GSpace
{
public:
	typedef pair<ObjectGeneratorType, ObjectIDType> generator_pair;

    GSpace(GScene* gscene);    
    ~GSpace();
    
	IntVec2 getSize() const;
    void setSize(int x, int y);
    
	unsigned int getFrame() const;
	GScene* getScene();

	template<class C>
	C* getSceneAs() {
		return dynamic_cast<C*>(getScene());
	}

    void update();
    void processAdditions();
private:
	//The graphics destination to use for all objects constructed in this space.
	GScene *const gscene;
    unsigned int frame = 1;
    IntVec2 spaceSize;
//BEGIN OBJECT MANIPULATION
public:
    static const bool logObjectArgs;
	static const set<type_index> trackedTypes;
	static const set<type_index> enemyTypes;

	void addWallBlock(const SpaceVect& ll, const SpaceVect& ur);
	void addWallBlock(const SpaceRect& area);

    gobject_ref createObject(const ValueMap& obj);
	gobject_ref createObject(ObjectGeneratorType factory);
	void createObjects(const ValueVector& objs);
        
    bool isValid(unsigned int uuid) const;
    vector<string> getObjectNames() const;
    unordered_map<int,string> getUUIDNameMap() const;
    inline int getObjectCount() const { return objByUUID.size();}
	unsigned int getAndIncrementObjectUUID();

    gobject_ref getObjectRef(const string& name) const;
    gobject_ref getObjectRef(unsigned int uuid) const;

	GObject* getObject(const string& name) const;
	GObject* getObject(unsigned int uuid) const;

	const set<GObject*>* getObjectsByType(type_index t) const;

    template<typename T>
    inline object_ref<T> getObjectRefAs(const string& name) const{
		assert_gobject(T);
		return object_ref<T>(getObject(name));
    }

	template<typename T>
	inline T* getObjectAs(const string& name) const {
		assert_gobject(T);
		return dynamic_cast<T*>(getObject(name));
	}

	template<typename T>
	inline vector<object_ref<T>> getObjectsByTypeAs() const {
		assert_gobject(T);
		const set<GObject*>* base = getObjectsByType(typeid(T));
		
		if (!base) return vector<object_ref<T>>();

		vector<object_ref<T>> result;
		result.reserve(base->size());

		for (GObject* basePtr : *base) {
			result.push_back(dynamic_cast<T*>(basePtr));
		}

		return result;
	}

	template<class TargetCls, class SenderCls, typename R, typename...Args>
	inline void messageAll(SenderCls* sender, R(TargetCls::*handler)(Args...), void (SenderCls::*response)(R), Args ...args)
	{
		const set<GObject*>* objects = getObjectsByType(typeid(TargetCls));

		for (GObject* objBase : *objects) {
			TargetCls* obj = dynamic_cast<TargetCls*>(objBase);
			if (obj) {
				obj->messageWithResponse(obj, sender, handler, response, args...);
			}
		}
	}

    void removeObject(const string& name);
    void removeObject(GObject* obj);

	void removeObjectWithAnimation(GObject* obj, ActionGeneratorType action);
    
	void setBulletBodiesVisible(bool b);

	void setInitialObjectCount();
	map<type_index, pair<unsigned int, unsigned int>> getEnemyStats();

	void addObjectAction(function<void()> f);
	void addSceneAction(pair<function<void(void)>, GScene::updateOrder> entry);
	void addSceneAction(function<void(void)> f, GScene::updateOrder order);

	void updatePlayerMapLocation(const SpaceVect& pos);
	void addMapArea(const SpaceRect& area);
	SpaceRect getCameraArea();
	const vector<SpaceRect>& getMapAreas();
	int getMapLocation(SpaceRect r);
	bool isInCameraArea(SpaceRect r);
	bool isInPlayerRoom(SpaceVect v);
	int getPlayerRoom();

	void teleportPlayerToDoor(string doorName);
	void setSuppressAction(bool b);
	bool getSuppressAction();
private:
    void processRemovals();
    void initObjects();
    void processRemoval(GObject* obj, bool removeSprite);
    
    unordered_map<unsigned int, GObject*> objByUUID;
    unordered_map<string, GObject*> objByName;
	unordered_set<string> warningNames;
	unordered_map<type_index, set<GObject*>> objByType;
	unordered_map<type_index, unsigned int> initialObjectCount;

	SpaceRect cameraArea;
	int crntMap = -1;
	vector<SpaceRect> mapAreas;
	
	vector<pair<function<void(void)>, GScene::updateOrder>> sceneActions;
	vector<function<void()>> objectActions;
	mutex objectActionsMutex;

	bool suppressAction = false;

	unsigned int nextObjUUID = 1;
    
    //"Objects" which have been queued for addition. The generator function, when added, is also
	//paired to a UUID, i.e. the UUID is actually determined when the object generator is added,
	//so that a ref can be returned in the same frame.
    vector<generator_pair> toAdd;
    //Objects whose additions have been processsed last frame. Physics has been initialized but
    //init has not yet run; it will run at start of frame.
    vector<GObject*> addedLastFrame;

    //Objects which have been queued for removal. Will be removed at end of frame.
    list<GObject*> toRemove;
	list<pair<GObject*, ActionGeneratorType>> toRemoveWithAnimation;
//END OBJECT MANIPULATION

//BEGIN GRAPHICS

public:
	//Since the scene controls the mapping of ID -> cocos2d::Node, these functions
	//will call the corresponding scene method immediately to retrieve the ID.
	//However, nextSpriteID/nextLightID is an atomic integer, so it will not use a mutex.

	LightID addLightSource(CircleLightArea light);
	LightID addLightSource(AmbientLightArea light);
	LightID addLightSource(ConeLightArea light);
	void updateLightSource(LightID id, ConeLightArea light);
	void removeLightSource(LightID id);
	void setLightSourcePosition(LightID id, SpaceVect pos);

	SpriteID createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	SpriteID createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	SpriteID createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	SpriteID createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom);

	void loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation);
	void setAgentAnimationDirection(SpriteID id, Direction d);
	void setAgentAnimationFrame(SpriteID id, int frame);

	void clearDrawNode(SpriteID id);
	void drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color);
	void drawSolidCone(SpriteID id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color);
	void drawSolidCircle(SpriteID id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color);

	void runSpriteAction(SpriteID id, ActionGeneratorType generator);
	void stopSpriteAction(SpriteID id, cocos_action_tag action);
	void stopAllSpriteActions(SpriteID id);
	void removeSprite(SpriteID id);
	void removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator);
	void setSpriteVisible(SpriteID id, bool val);
	void setSpriteOpacity(SpriteID id, unsigned char op);
	void setSpriteTexture(SpriteID id, string path);
	void setSpriteAngle(SpriteID id, float cocosAngle);
	void setSpritePosition(SpriteID id, Vec2 pos);
	void setSpriteZoom(SpriteID id, float zoom);
protected:
	void addLightmapAction(function<void()> f);
	void addSpriteAction(function<void()> f);

//END GRAPHICS


//BEGIN NAVIGATION
public:
    bool isObstacle(IntVec2) const;
    void addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);

    vector<SpaceVect> pathToTile(IntVec2 begin, IntVec2 end);
    void addPath(string name, Path p);
	const Path* getPath(string name) const;

	void addWaypoint(string name, SpaceVect w);
	SpaceVect getWaypoint(string name) const;

	FloorSegment* floorSegmentPointQuery(SpaceVect pos);

    inline boost::dynamic_bitset<>* getNavMask() const { return navMask;}
private:
    void markObstacleTile(int x, int y);
    bool isObstacleTile(int x, int y) const;
    
	unordered_map<string, Path> paths;
	unordered_map<string, SpaceVect> waypoints;
    boost::dynamic_bitset<>* navMask = nullptr;
//END NAVIGATION
    
//BEGIN PHYSICS
public:
	typedef pair<GObject*, GObject*> object_pair;
	typedef pair<GType, GType> collision_type;
	typedef pair<object_pair, collision_type> contact;

    static const bool logBodyCreation;
    static const bool logPhysicsHandlers;
    
   pair<cpShape*,cpBody*> createCircleBody(
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
private:
    cpSpace *space = nullptr;

	unordered_map<GObject*,list<contact>> currentContacts;
	unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*), boost::hash<collision_type>> beginContactHandlers;
	unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*), boost::hash<collision_type>> endContactHandlers;

	void addContact(contact c);
	void removeContact(contact c);
    void addCollisionHandlers();
    void processRemovalEndContact(GObject* obj);
	void processContactHandler(GType typeA, GType typeB, GObject* a, GObject* b);
    
	static inline int beginContact(cpArbiter* arb, cpSpace* space, void* data)
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
		if(it == _this->beginContactHandlers.end())
			return 0;

		if (a && b && it->second) {
			int(GSpace::*begin_method)(GObject*, GObject*) = it->second;
			(_this->*begin_method)(a, b);
			contact c = contact(
				object_pair(a,b),
				collision_type(typeA,typeB)
			);
			_this->addContact(c);
		}

		return 1;
	}

	static inline void endContact(cpArbiter* arb, cpSpace* space, void* data)
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
			int(GSpace::*end_method)(GObject*, GObject*) = it->second;
			(_this->*end_method)(a, b);
			contact c = contact(
				object_pair(a, b),
				collision_type(typeA, typeB)
			);
			_this->removeContact(c);
		}
	}

	template<GType TypeA, GType TypeB>
	inline void AddHandler(int(GSpace::*begin)(GObject*, GObject*), int(GSpace::*end)(GObject*, GObject*))
	{
		cpSpaceAddCollisionHandler(
			space,
			to_uint(TypeA),
			to_uint(TypeB),
			&GSpace::beginContact,
			static_cast<cpCollisionPreSolveFunc>(nullptr),
			static_cast<cpCollisionPostSolveFunc>(nullptr),
			&GSpace::endContact,
			this
		);

		beginContactHandlers[collision_type(TypeA, TypeB)] = begin;
		endContactHandlers[collision_type(TypeA,TypeB)] = end;
	}
    
    void logHandler(const string& base, cpArbiter* arb);
    void logHandler(const string& name, GObject* a, GObject* b);
    
	int playerEnemyBegin(GObject* a, GObject* b);
	int playerEnemyEnd(GObject* a, GObject* b);
	int playerEnemyBulletBegin(GObject* playerObj, GObject* bullet);
	int playerGrazeRadarBegin(GObject* playerObj, GObject* bullet);
	int playerGrazeRadarEnd(GObject* playerObj, GObject* bullet);
	int playerBulletEnemyBegin(GObject* a, GObject* b);
	int bulletBulletBegin(GObject* a, GObject* b);
	int agentEffectAreaBegin(GObject* a, GObject* b);
	int agentEffectAreaEnd(GObject* a, GObject* b);
	int playerFlowerBegin(GObject* a, GObject* b);
    int playerCollectibleBegin(GObject* a, GObject* b);
	int playerUpgradeBegin(GObject* a, GObject* b);
	int bulletEnvironment(GObject* a, GObject* b);
	int noCollide(GObject* a, GObject* b);
	int collide(GObject* a, GObject* b);
	int bulletWall(GObject* bullet, GObject* unused);
	int sensorStart(GObject* radarAgent, GObject* target);
	int sensorEnd(GObject* radarAgent, GObject* target);
	int floorObjectBegin(GObject* floorSegment, GObject* obj);
	int floorObjectEnd(GObject* floorSegment, GObject* obj);
	int teleportPadObjectBegin(GObject* teleportPad, GObject* obj);
	int teleportPadObjectEnd(GObject* teleportPad, GObject* obj);
	int spawnerObjectBegin(GObject* spawner, GObject* obj);
	int spawnerObjectEnd(GObject* spawner, GObject* obj);
	int playerAreaSensorBegin(GObject* a, GObject *b);
	int playerAreaSensorEnd(GObject* a, GObject *b);
	int enemyAreaSensorBegin(GObject* a, GObject *b);
	int enemyAreaSensorEnd(GObject* a, GObject *b);

//END PHYSICS

//BEGIN SENSORS
public:
	GObject * pointQuery(SpaceVect pos, GType type, PhysicsLayers layers);
//	bool rectangleQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers);

    SpaceFloat distanceFeeler(const GObject * agent, SpaceVect feeler, GType gtype) const;
	SpaceFloat distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const;

    SpaceFloat obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler) const;
    SpaceFloat wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const;
	SpaceFloat trapFloorDistanceFeeler(const GObject* agent, SpaceVect feeler) const;

    bool feeler(const GObject * agent, SpaceVect feeler, GType gtype) const;
    bool feeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const;
	GObject* objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const;
	bool obstacleFeeler(const GObject * agent, SpaceVect feeler) const;
    bool wallFeeler(const GObject * agent, SpaceVect feeler) const;
	InteractibleObject* interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const;

    bool lineOfSight(const GObject * agent, const GObject * target) const;
//END SENSORS
};

#endif /* GSpace_hpp */
