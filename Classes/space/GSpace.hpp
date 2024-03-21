//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

template<class D>
constexpr bool isObjectCls() {
	return is_base_of<GObject, D>();
}

#define assert_gobject(cls) static_assert( isObjectCls<cls>(),"Not a GObject type!");
typedef pair<object_params, local_shared_ptr<object_properties>> object_generator;

class GSpace
{
public:
	friend class GScene;
	friend class node_context;
	friend class physics_context;
	friend class PhysicsImpl;

	static void loadScriptVM();
    static GSpace* getCrntSpace();

    static const vector<string> scriptFiles;
	static unique_ptr<Lua::Inst> scriptVM;
	static local_shared_ptr<agent_properties> playerCharacter;
    static GSpace* crntSpace;

    GSpace(GScene* gscene);    
    ~GSpace();
    
	IntVec2 getSize() const;
    void setSize(int x, int y);
    
	AreaStats& getAreaStats();
	local_shared_ptr<area_properties> getCrntArea();
	physics_context* getPhysics() const { return physicsContext.get(); }
	inline bool isInCallback() const { return isInPhysicsStep; }
    inline bool getIsUnloading() const { return isUnloading; }
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

	void addSpatialSound(GObject* sourceObj, ALuint soundSource);
	void removeSpatialSound(ALuint soundSource);
	void removeSpatialSounds(GObject* sourceObj);

	void addWallBlock(const SpaceVect& ll, const SpaceVect& ur);
	void addWallBlock(const SpaceRect& area);

	void addDynamicLoadObject(const ValueMap& obj);
	gobject_ref createDynamicObject(const string& name);
	const object_generator* getDynamicObject(const string& name) const;
    GObject* createObject(const object_params& params, local_shared_ptr<object_properties> props);
	gobject_ref createObject(const ValueMap& obj);
	Bullet* createBullet(
		const object_params& params,
		const bullet_attributes& attributes,
		local_shared_ptr<bullet_properties> props
	);
        
	template<class C, typename... Args>
	inline C* createObject(Args... args) { 
        C* obj = new C(this, nextObjUUID++, args...);
        addObject(obj);
		return obj;
	}

	bool isTrackedType(type_index t) const;
    bool isValid(unsigned int uuid) const;
	bool isFutureObject(ObjectIDType uuid) const;
    inline int getObjectCount() const { return objByUUID.size();}

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
	void createDialog(string res, bool autoAdvance);
	void createDialog(string res, bool autoAdvance, zero_arity_function f);
    void loadScene(string mapName, string start);

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
    bool getRandomBool();
    //probability of getting a true result.
    bool getRandomBool(float probability);
	//Generate the numbers [0,N) in a random order
	vector<int> getRandomShuffle(int n);

	ControlInfo getControlInfo() const;
	void setControlInfo(ControlInfo info);
private:
    void addObject(GObject* obj);
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
	template<typename... Args>
	inline void addGraphicsAction(void (graphics_context::*m)(Args...), Args... args)
	{
		sceneActions.push_back(bind(m, graphicsContext, args...));
	}

//END GRAPHICS

//BEGIN NAVIGATION
public:
	bool isObstacleObject(const GObject* obj) const;
    bool isObstacle(IntVec2) const;
    void addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);
	void removeNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);

    shared_ptr<const Path> pathToTile(IntVec2 begin, IntVec2 end);
    void addPath(string name, Path p);
	shared_ptr<const Path> getPath(string name) const;

	void addWaypoint(string name, SpaceVect w);
	void addWaypoint(string name, const vector<string>& tags, SpaceVect w);
	SpaceVect getWaypoint(string name) const;
	const list<SpaceVect>* getWaypoints(string name) const;
	SpaceVect getRandomWaypoint(string name);

	void addArea(string name, SpaceRect a);
	SpaceRect getArea(string name) const;

    FloorSegment* floorPointQuery(SpaceVect pos);
	Pitfall* pitfallPointQuery(SpaceVect pos);

    inline const boost::dynamic_bitset<>* getNavMask() const { return navMask;}
//END NAVIGATION    

	audio_context* audioContext;
	unique_ptr<physics_context> physicsContext;

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
	SpaceFloat elapsedTime = 0.0;
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

	//Objects which have been queued for addition.
	vector<GObject*> toAdd;
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
    unordered_map<string, GObject*> objByName;
	unordered_map<type_index, unordered_set<GObject*>> objByType;
	
	set<GObject*> updateObjects;
	set<RadarSensor*> radarSensors;
	unordered_map<int, RoomSensor*> roomSensors;
	unordered_map<string, object_generator> dynamicLoadObjects;
//NAVIGATION
	void unmarkObstacleTile(int x, int y);
	void markObstacleTile(int x, int y);
    bool isObstacleTile(int x, int y) const;
    
	unordered_map<string, shared_ptr<const Path>> paths;
	unordered_map<string, list<SpaceVect>> waypoints;
	unordered_map<string, SpaceRect> areas;
	boost::dynamic_bitset<>* navMask = nullptr;
};

#endif /* GSpace_hpp */
