//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include "controls.h"
#include "graphics_types.h"
#include "macros.h"
#include "object_ref.hpp"

class FloorSegment;
class GObject;
class GScene;
class GState;
class HUD;
class InteractibleObject;
class PlayScene;
class Replay;

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
	friend class GScene;
	typedef pair<ObjectGeneratorType, ObjectIDType> generator_pair;

    GSpace(GScene* gscene);    
    ~GSpace();
    
	//Will return this GSpace's state if running a chamber scene, else the App::crntState.
	GState* getState();

	IntVec2 getSize() const;
    void setSize(int x, int y);
    
	unsigned int getFrame() const;
	unsigned long getTimeUsed() const;
	GScene* getScene();

	template<class C>
	C* getSceneAs() {
		return dynamic_cast<C*>(getScene());
	}

    void update();
    void processAdditions();
private:
	unique_ptr<GState> crntState;
	ChamberID crntChamber;
	//The graphics destination to use for all objects constructed in this space.
	GScene *const gscene;
    unsigned int frame = 0;
    IntVec2 spaceSize;
	unsigned long timeUsed = 0;
//BEGIN OBJECT MANIPULATION
public:
    static const bool logObjectArgs;
	static const unordered_set<type_index> trackedTypes;
	static const unordered_set<type_index> enemyTypes;

	void addWallBlock(const SpaceVect& ll, const SpaceVect& ur);
	void addWallBlock(const SpaceRect& area);

    gobject_ref createObject(const ValueMap& obj);
	gobject_ref createObject(ObjectGeneratorType factory);
	void createObjects(const ValueVector& objs);
        
	bool isTrackedType(type_index t) const;
    bool isValid(unsigned int uuid) const;
	bool isFutureObject(ObjectIDType uuid) const;
    vector<string> getObjectNames() const;
    unordered_map<int,string> getUUIDNameMap() const;
    inline int getObjectCount() const { return objByUUID.size();}
	unsigned int getAndIncrementObjectUUID();

    gobject_ref getObjectRef(const string& name) const;
    gobject_ref getObjectRef(unsigned int uuid) const;

	GObject* getObject(const string& name) const;
	GObject* getObject(unsigned int uuid) const;

	const unordered_set<GObject*>* getObjectsByType(type_index t) const;

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
		const unordered_set<GObject*>* base = getObjectsByType(typeid(T));
		
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
		const unordered_set<GObject*>* objects = getObjectsByType(typeid(TargetCls));

		for (GObject* objBase : *objects) {
			if (static_cast<GObject*>(sender) == objBase) continue;

			TargetCls* obj = dynamic_cast<TargetCls*>(objBase);
			if (obj) {
				obj->messageWithResponse(obj, sender, handler, response, args...);
			}
		}
	}

    void removeObject(const string& name);
    void removeObject(GObject* obj);
	void removeObject(gobject_ref ref);

	void removeObjectWithAnimation(GObject* obj, ActionGeneratorType action);
    
	void setBulletBodiesVisible(bool b);

	void setInitialObjectCount();
	EnemyStatsMap getEnemyStats();

	template<typename ...Args>
	inline void addHudAction(void (HUD::*m)(Args...), Args... args)
	{
		if (!getSceneAs<PlayScene>()) return;

		addSceneAction(make_hud_action(
			m,
			getSceneAs<PlayScene>(),
			args...
		));
	}

	void addObjectAction(zero_arity_function f);
	void addSceneAction(zero_arity_function f);
	void createDialog(string res, bool autoAdvance);
	void createDialog(string res, bool autoAdvance, zero_arity_function f);

	void eraseTile(const SpaceVect& p, string layer);
	void eraseTile(int mapID, IntVec2 pos, string layer);
	void updatePlayerMapLocation(const SpaceVect& pos);
	void addMapArea(const SpaceRect& area);
	SpaceRect getCameraArea();
	const vector<SpaceRect>& getMapAreas();
	int getMapLocation(SpaceRect r);
	pair<int, IntVec2> getTilePosition(SpaceVect p);
	bool isInCameraArea(SpaceRect r);
	bool isInPlayerRoom(SpaceVect v);
	int getPlayerRoom();

	void teleportPlayerToDoor(string doorName);
	void setSuppressAction(bool b);
	bool getSuppressAction();

	void setRandomSeed(unsigned int seed);
	//Generate [0,1)
	float getRandomFloat();
	//Generate [min,max)
	float getRandomFloat(float min, float max);
	//Generate [min,max]
	int getRandomInt(int min, int max);

	void loadReplay(unique_ptr<Replay> replay);
	inline const Replay* getReplay() { return controlReplay.get(); }
	ControlInfo getControlInfo() const;
	void setControlInfo(ControlInfo info);
	inline bool getIsRunningReplay() { return isRunningReplay; }
private:
	void updateControlInfo();
    void processRemovals();
    void initObjects();
    void processRemoval(GObject* obj, bool removeSprite);
	bool isNoUpdateObject(GObject* obj);
    
	template<class C>
	inline void addVirtualTrack(GObject* obj)
	{
		if (dynamic_cast<C*>(obj)) {
			objByType[typeid(C)].insert(obj);
		}
	}

	template<class C>
	inline void removeVirtualTrack(GObject* obj)
	{
		if (dynamic_cast<C*>(obj)) {
			objByType[typeid(C)].erase(obj);
		}
	}

    unordered_map<unsigned int, GObject*> objByUUID;
    unordered_map<string, GObject*> objByName;
	unordered_set<string> warningNames;
	unordered_map<type_index, unordered_set<GObject*>> objByType;
	unordered_map<type_index, unsigned int> initialObjectCount;
	unordered_set<GObject*> updateObjects;

	SpaceRect cameraArea;
	int crntMap = -1;
	vector<SpaceRect> mapAreas;

	ControlInfo controlInfo;
	unique_ptr<Replay> controlReplay;
	
	vector<zero_arity_function> sceneActions;
	vector<zero_arity_function> objectActions;
	mutex objectActionsMutex;

	bool isRunningReplay = false;
	bool suppressAction = false;
	bool isMultiMap;

	boost::random::uniform_01<float> randomFloat;
	boost::random::uniform_int_distribution<int> randomInt;
	boost::random::mt19937 randomEngine;

	unsigned int nextObjUUID = 1;
	unsigned int lastAddedUUID = 0;
    
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

	template<typename T>
	inline LightID addLightSource(T light)
	{
		LightID id = gscene->getLightID();

		sceneActions.push_back([this, id, light]()->void {
			gscene->addLightSource(id, light);
		});
		return id;
	}

	template<typename... Args>
	inline void addLightmapAction(void (GScene::*m)(Args...), Args... args)
	{
		sceneActions.push_back(bind(m, gscene, args...));
	}

	template<typename... Args>
	inline SpriteID createSprite(void (GScene::*m)(SpriteID, Args...), Args... args)
	{
		SpriteID id = gscene->getSpriteID();

		sceneActions.push_back(bind(m, gscene, id, args...));

		return id;
	}

	template<typename... Args>
	inline void addSpriteAction(void (GScene::*m)(Args...), Args... args)
	{
		sceneActions.push_back(bind(m, gscene, args...));
	}

	void removeLightSource(LightID id);
	void setLightSourcePosition(LightID id, SpaceVect pos);
	void setLightSourceAngle(LightID id, SpaceFloat a);
	void setLightSourceColor(LightID id, Color4F color);

	SpriteID createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	SpriteID createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	SpriteID createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	SpriteID createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createDamageIndicator(float val, SpaceVect pos);
	SpriteID createAgentBodyShader(
		GraphicsLayer layer,
		const Color4F& bodyColor, const Color4F& coneColor,
		float bodyRadius, float coneRadius,
		float thickness, const Vec2& position,
		float startAngle, float endAngle
	);

	void loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation);
	void setAgentAnimationDirection(SpriteID id, Direction d);
	void setAgentAnimationFrame(SpriteID id, int frame);

	void clearDrawNode(SpriteID id);
	void drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color);
	void drawSolidCone(SpriteID id, Vec2 center, float radius, float startAngle, float endAngle, unsigned int segments, Color4F color);
	void drawSolidCircle(SpriteID id, Vec2 center, float radius, float angle, unsigned int segments, Color4F color);

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
	void setSpriteColor(SpriteID id, Color3B color);
	void setAgentOverlayAngles(SpriteID id, float startAngle, float endAngle);

	void clearSubroomMask(unsigned int roomID);

//END GRAPHICS

//BEGIN NAVIGATION
public:
    bool isObstacle(IntVec2) const;
    void addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);
	void removeNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);

    vector<SpaceVect> pathToTile(IntVec2 begin, IntVec2 end);
    void addPath(string name, Path p);
	const Path* getPath(string name) const;

	void addWaypoint(string name, SpaceVect w);
	SpaceVect getWaypoint(string name) const;

	FloorSegment* floorSegmentPointQuery(SpaceVect pos);

    inline boost::dynamic_bitset<>* getNavMask() const { return navMask;}
private:
	void unmarkObstacleTile(int x, int y);
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
	static int beginContact(cpArbiter* arb, cpSpace* space, void* data);
	static void endContact(cpArbiter* arb, cpSpace* space, void* data);

    cpSpace *space = nullptr;

	unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*, cpArbiter*), boost::hash<collision_type>> beginContactHandlers;
	unordered_map<collision_type, void(GSpace::*)(GObject*, GObject*, cpArbiter*), boost::hash<collision_type>> endContactHandlers;

    void addCollisionHandlers();
    
	template<GType TypeA, GType TypeB>
	inline void AddHandler(int(GSpace::*begin)(GObject*, GObject*, cpArbiter*), void(GSpace::*end)(GObject*, GObject*, cpArbiter*))
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
		
		if(begin)
			beginContactHandlers[collision_type(TypeA, TypeB)] = begin;
		if(end)
			endContactHandlers[collision_type(TypeA,TypeB)] = end;
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
    int playerCollectibleBegin(GObject* a, GObject* b, cpArbiter* arb);
	int playerUpgradeBegin(GObject* a, GObject* b, cpArbiter* arb);
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

//END PHYSICS

//BEGIN SENSORS
public:
	static const GType interactibleObjects;
	static const GType obstacles;

	GObject * queryAdjacentTiles(SpaceVect pos, GType type, PhysicsLayers layers, type_index t);
	GObject * pointQuery(SpaceVect pos, GType type, PhysicsLayers layers);
	bool rectangleQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle = 0.0);
	SpaceFloat rectangleFeelerQuery(const GObject* agent, SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle) const;
	unordered_set<GObject*> rectangleObjectQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle = 0.0);
	bool obstacleRadiusQuery(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers);
	unordered_set<GObject*> radiusQuery(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers);

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
//END SENSORS
};

#endif /* GSpace_hpp */
