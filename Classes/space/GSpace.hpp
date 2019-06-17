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
#include "object_ref.hpp"

class audio_context;
class FloorSegment;
class GObject;
class graphics_context;
class GScene;
class GState;
class HUD;
class InteractibleObject;
class PlayScene;
class Replay;
class RoomSensor;

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
	friend class physics_context;
	friend class PhysicsImpl;
	typedef pair<ObjectGeneratorType, ObjectIDType> generator_pair;

    GSpace(GScene* gscene);    
    ~GSpace();
    
	//Will return this GSpace's state if running a chamber scene, else the App::crntState.
	GState* getState();

	IntVec2 getSize() const;
    void setSize(int x, int y);
    
	inline ChamberID getCrntChamber() const { return crntChamber; }
	unsigned int getFrame() const;
	unsigned long getTimeUsed() const;
	GScene* getScene();

	template<class C>
	C* getSceneAs() {
		return dynamic_cast<C*>(getScene());
	}

    void update();
    void processAdditions();

	audio_context* audioContext;
	unique_ptr<physics_context> physicsContext;
private:
	unique_ptr<GState> crntState;
	ChamberID crntChamber;

	//The graphics destination to use for all objects constructed in this space.
	GScene *const gscene;
	graphics_context* graphicsContext;
    
	cpSpace *space = nullptr;
	unique_ptr<PhysicsImpl> physicsImpl;
	
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

	void addDynamicLoadObject(const ValueMap& obj);
	gobject_ref createDynamicObject(const string& name);
	const ValueMap* getDynamicObject(const string& name) const;
	gobject_ref createObject(const ValueMap& obj);
	gobject_ref createObject(ObjectGeneratorType factory);
        
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
	RoomSensor* getRoomSensor(int id) const;

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
	inline vector<T*> getObjectsAs(const vector<string>& names) const {
		assert_gobject(T);
		vector<T*> result;

		for (string s : names)
		{
			T* obj = getObjectAs<T>(s);
			if (obj) result.push_back(obj);
		}

		return result;
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
	void applyMapFragment(int mapFragmentID);
	void increaseSpawnTotal(type_index t, unsigned int count);
	void registerEnemyDefeated(type_index t);

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
	//Generate the numbers [0,N) in a random order
	vector<int> getRandomShuffle(int n);

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
	unordered_map<type_index, unsigned int> totalSpawnCount;
	unordered_map<type_index, unsigned int> enemiesDefeated;
	set<GObject*> updateObjects;
	unordered_map<int, RoomSensor*> roomSensors;
	unordered_map<string, ValueMap> dynamicLoadObjects;

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

	uniform_real_distribution<float> randomFloat;
	mt19937 randomEngine;

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

	inline LightID addLightSource(shared_ptr<LightArea> light)
	{
		LightID id = graphicsContext->getLightID();

		sceneActions.push_back(bind(
			&graphics_context::addPolyLightSource,
			graphicsContext,
			id,
			light
		));

		return id;
	}

	template<typename... Args>
	inline void addLightmapAction(void (graphics_context::*m)(Args...), Args... args)
	{
		sceneActions.push_back(bind(m, graphicsContext, args...));
	}

	template<typename... Args>
	inline SpriteID createSprite(void (graphics_context::*m)(SpriteID, Args...), Args... args)
	{
		SpriteID id = graphicsContext->getSpriteID();

		sceneActions.push_back(bind(m, graphicsContext, id, args...));

		return id;
	}

	template<class C, typename... Params, typename... Args>
	void graphicsNodeAction(void (C::*method)(Params...), SpriteID id, Args... args)
	{
		sceneActions.push_back([this, id, method, args...]() -> void {
			graphicsContext->nodeAction(id, method, args...);
		});
	}

	template<typename... Args>
	inline void addGraphicsAction(void (graphics_context::*m)(Args...), Args... args)
	{
		sceneActions.push_back(bind(m, graphicsContext, args...));
	}

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
};

#endif /* GSpace_hpp */
