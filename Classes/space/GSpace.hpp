//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

class audio_context;
struct bullet_attributes;
struct bullet_properties;
struct ChamberStats;
class FloorSegment;
class GObject;
class graphics_context;
class GScene;
class GState;
class HUD;
namespace Lua { class Inst; }
class MagicEffectSystem;
class Pitfall;
class PlayScene;
class RadarSensor;
class RoomSensor;
class SpellSystem;

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
	friend class SpellSystem;
	typedef pair<ObjectGeneratorType, ObjectIDType> generator_pair;

	static void loadScriptVM();

	static unique_ptr<Lua::Inst> scriptVM;
	static local_shared_ptr<agent_properties> playerCharacter;

    GSpace(GScene* gscene);    
    ~GSpace();
    
	IntVec2 getSize() const;
    void setSize(int x, int y);
    
	ChamberStats& getCrntChamberStats();
	inline string getCrntChamber() const { return crntChamber; }
	inline bool isInCallback() const { return isInPhysicsStep; }
	unsigned int getFrame() const;
	SpaceFloat getTime() const;
	unsigned long getTimeUsed() const;
	GScene* getScene();

	template<class C>
	C* getSceneAs() {
		return dynamic_cast<C*>(getScene());
	}

    void update();
	void updateSensors();
	void updateSoundSources();
    void processAdditions();

//BEGIN OBJECT MANIPULATION
public:
    static const bool logObjectArgs;
	static const unordered_set<type_index> trackedTypes;

	void addSpatialSound(GObject* sourceObj, ALuint soundSource);
	void removeSpatialSound(ALuint soundSource);
	void removeSpatialSounds(GObject* sourceObj);

	void addWallBlock(const SpaceVect& ll, const SpaceVect& ur);
	void addWallBlock(const SpaceRect& area);

	void addDynamicLoadObject(const ValueMap& obj);
	gobject_ref createDynamicObject(const string& name);
	const ValueMap* getDynamicObject(const string& name) const;
	gobject_ref createObject(const ValueMap& obj);
	gobject_ref createObject(ObjectGeneratorType factory);
	gobject_ref createBullet(
		const object_params& params,
		const bullet_attributes& attributes,
		local_shared_ptr<bullet_properties> props
	);

	inline void addValueMapArgs(ObjectIDType id, const ValueMap& args) {
		valueMapArgs.insert_or_assign(id, args);
	}

	inline void removeValueMapArgs(ObjectIDType id) {
		valueMapArgs.erase(id);
	}

	inline const ValueMap& getValueMapArgs(ObjectIDType id) {
		return valueMapArgs.at(id);
	}
        
	template<class C, typename... Args>
	inline gobject_ref createObject(Args... args) {
		return createObject(GObject::make_object_factory<C>(args...));
	}

	bool isTrackedType(type_index t) const;
    bool isValid(unsigned int uuid) const;
	bool isFutureObject(ObjectIDType uuid) const;
    inline int getObjectCount() const { return objByUUID.size();}
	unsigned int getAndIncrementObjectUUID();

    gobject_ref getObjectRef(const string& name) const;
    gobject_ref getObjectRef(unsigned int uuid) const;

	GObject* getObject(const string& name) const;
	GObject* getObject(unsigned int uuid) const;
	RoomSensor* getRoomSensor(int id) const;

	const unordered_set<GObject*>* getObjectsByType(type_index t) const;

	string getObjectName(ObjectIDType id) const;

	template<typename T>
	inline T* getObjectAs(const string& name) const {
		assert_gobject(T);
		return dynamic_cast<T*>(getObject(name));
	}

	template<typename T>
	inline vector<T*> getObjectsAs(const vector<string>& names) const {
		assert_gobject(T);
		vector<T*> result;
		result.reserve(names.size());

		for (string s : names)
		{
			T* obj = getObjectAs<T>(s);
			if (obj) result.push_back(obj);
		}

		return result;
	}

	template<typename T>
	inline vector<gobject_ref> getObjectsByTypeAs() const {
		assert_gobject(T);
		const unordered_set<GObject*>* base = getObjectsByType(typeid(T));
		
		if (!base) return vector<gobject_ref>();

		vector<gobject_ref> result;
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

	inline void insertSensor(RadarSensor* sensor) { radarSensors.insert(sensor); }
	inline void removeSensor(RadarSensor* sensor) { radarSensors.erase(sensor); }

	gobject_ref createAreaSensor(
		SpaceRect rect,
		GType targets,
		unary_gobject_function onContact,
		unary_gobject_function onEndContact
	);

	void addInitAction(zero_arity_function f);
	void addUpdateAction(zero_arity_function f);
	void addObjectAction(zero_arity_function f);
	void addSceneAction(zero_arity_function f);
	void enterWorldSelect();
	void triggerSceneCompleted();
	void createDialog(string res, bool autoAdvance);
	void createDialog(string res, bool autoAdvance, zero_arity_function f);

	void eraseTile(const SpaceVect& p, string layer);
	void eraseTile(int mapID, IntVec2 pos, string layer);
	void updatePlayerMapLocation(int roomID);
	void addMapArea(const SpaceRect& area);
	void registerRoomMapped(int roomID);
	SpaceRect getCameraArea();
	void updateCamera();
	void setCameraPosition(SpaceVect pos);
	const vector<SpaceRect>& getMapAreas();
	int getMapLocation(SpaceRect r);
	pair<int, IntVec2> getTilePosition(SpaceVect p);
	bool isInCameraArea(SpaceRect r);
	bool isInPlayerRoom(SpaceVect v);
	bool isInPlayerRoom(int roomID);
	int getPlayerRoom();
	Player* getPlayer();
	gobject_ref getPlayerAsRef();
	void addRoomSensor(RoomSensor* rs);
	void removeRoomSensor(RoomSensor* rs);

	void registerEnemyStaticLoaded(string s);
	void registerEnemySpawned(string s);
	void increasePotentialSpawnTotal(string t, unsigned int count);
	void registerEnemyDefeated(string t);

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

	ControlInfo getControlInfo() const;
	void setControlInfo(ControlInfo info);
private:
    void processRemovals();
    void initObjects();
    void processRemoval(GObject* obj, bool removeSprite);
	bool isNoUpdateObject(GObject* obj);

//END OBJECT MANIPULATION

//BEGIN GRAPHICS

public:
	//Since the scene controls the mapping of ID -> cocos2d::Node, these functions
	//will call the corresponding scene method immediately to retrieve the ID.
	//However, nextSpriteID/nextLightID is an atomic integer, so it will not use a mutex.

	LightID addLightSource(shared_ptr<LightArea> light, SpaceVect pos, SpaceFloat angle);

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

	void addArea(string name, SpaceRect a);
	SpaceRect getArea(string name) const;

	Pitfall* pitfallPointQuery(SpaceVect pos);

    inline boost::dynamic_bitset<>* getNavMask() const { return navMask;}
//END NAVIGATION    

	audio_context* audioContext;
	unique_ptr<physics_context> physicsContext;
	unique_ptr<MagicEffectSystem> magicEffectSystem;
	unique_ptr<SpellSystem> spellSystem;

protected:
//LOGIC
	ControlInfo controlInfo;
	b2World *world = nullptr;
	unique_ptr<PhysicsImpl> physicsImpl;
	GScene *const gscene;
	//This should be protected, since objects are not supposed to use it directly, 
	//but rather make function-bind messages from its methods.
	graphics_context* graphicsContext;

	SpaceRect cameraArea;
	IntVec2 spaceSize;
	unsigned int frame = 0;
	unsigned long timeUsed = 0;
	string crntChamber;
	int crntMap = -1;

	list<pair<GObject*, ALuint>, local_allocator<pair<GObject*, ALuint>>> activeSounds;
	vector<SpaceRect> mapAreas;

	bool suppressAction = false;
	bool isMultiMap;
	bool isInPhysicsStep = false;
	bool isUnloading = false;

//OBJECT MANIPULATION

	vector<zero_arity_function> sceneActions;
	vector<zero_arity_function> objectActions;
	mutex objectActionsMutex;

	uniform_real_distribution<float> randomFloat;
	mt19937 randomEngine;

	unsigned int nextObjUUID = 1;
	unsigned int lastAddedUUID = 0;

	//"Objects" which have been queued for addition. The generator function, when added, is also
	//paired to a UUID, i.e. the UUID is actually determined when the object generator is added,
	//so that a ref can be returned in the same frame.
	vector<generator_pair> toAdd;
	//Messages for objects that have been queued for addition on the next frame. 
	//These will be run right after init is run for recently created objects.
	vector<zero_arity_function> initMessages;
	//Messages for objects that will be run before the next update cycle.
	vector<zero_arity_function> updateMessages;

	//Objects whose additions have been processsed last frame. Physics has been initialized but
	//init has not yet run; it will run at start of frame.
	vector<GObject*> addedLastFrame;
	//Objects which have been queued for removal. Will be removed at end of frame.
	list<GObject*, local_allocator<GObject*>> toRemove;
	list<pair<GObject*, ActionGeneratorType>, local_allocator<pair<GObject*, ActionGeneratorType>>> toRemoveWithAnimation;

	unordered_map<ObjectIDType, GObject*> objByUUID;
	boost::bimap<string, ObjectIDType> objectNames;
	unordered_map<type_index, unordered_set<GObject*>> objByType;

	set<string> enemyTypes;
	unordered_map<string, unsigned int> initialEnemyCount;
	unordered_map<string, unsigned int> actualSpawnCount;
	unordered_map<string, unsigned int> potentialSpawnCount;
	unordered_map<string, unsigned int> enemiesDefeated;
	
	set<GObject*> updateObjects;
	set<RadarSensor*> radarSensors;
	unordered_map<int, RoomSensor*> roomSensors;
	unordered_map<string, ValueMap> dynamicLoadObjects;
	//For objects that actually need ValueMap args to persist until init,
	//without having to store them by value in the object itself.
	unordered_map<ObjectIDType, ValueMap> valueMapArgs;
//NAVIGATION
	void unmarkObstacleTile(int x, int y);
	void markObstacleTile(int x, int y);
    bool isObstacleTile(int x, int y) const;
    
	unordered_map<string, Path> paths;
	unordered_map<string, SpaceVect> waypoints;
	unordered_map<string, SpaceRect> areas;
	boost::dynamic_bitset<>* navMask = nullptr;
};

#endif /* GSpace_hpp */
