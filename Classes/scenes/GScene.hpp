//
//  GScene.hpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#ifndef GScene_hpp
#define GScene_hpp

#include "multifunction.h"

class AgentBodyShader;
class ConeShader;
class ControlListener;
class Dialog;
class GAnimation;
class GSpace;
class HUD;
class LuaShell;
class MenuLayer;
class PatchConAnimation;
class PlayScene;
class RadialGradient;

namespace Lua{
    class Inst;
}

typedef vector<vector<int>> MapFragmentsList;

class GScene : public Scene
{
public:
	friend class graphics_context;

    //The order of initialization events, as performed by GScene and it's various dervied classes.
    enum class initOrder{
        //This includes all top-level GScene init, as well running Layer::init and Node::scheduleUpdate.
        core = 1,
        //Loading map objects if applicable
        mapLoad,
		//For PlayScene, if area has already been visited and rooms are marked visible in App state,
		//initialize with rooms visible.
		showRooms,
        //Running GSpace::loadAdditions, if applicable
        loadObjects,
        initHUD,
		postInitHUD,
        //Objects that wish to query the GSpace, including looking up other objects that are expected
        //to be loaded.
        postLoadObjects,
    };
    
    enum class sceneLayers{
        //Represents the world and objects in it (map and all gspace objects).
        //GraphicsLayer is used for z-ordering inside of this layer.
        //It is the only layer that moves with the camera
		begin = 1,
        space = 1,
		lightmapBackground,
		lightmap,
		screenspaceColorFilter,
        dialogBackground,
        dialog,
        hud,
        menu,
        luaShell,
        end
    };

	enum class displayMode {
		begin = 0,
		base = 0,
		lightmap,
		combined,

		end
	};

    typedef function<GScene* () > AdapterType;
	typedef pair<string, IntVec2> MapEntry;

    //Map each class name to a constructor adapter function.
	//Defined in SceneMapping.cpp.
    static const unordered_map<string,AdapterType> adapters;

	static const Color4F defaultAmbientLight;
	static const int dialogEdgeMargin;
	static const bool scriptLog;

	static string crntSceneName;
	static bool suppressGameOver;

	static GScene* runScene(const string& name);
	static void restartScene();

	vector<MapEntry> singleMapEntry(const string& mapName);
	static const IntVec2 getRoomOffset(IntVec2 roomSize, int roomGridX, int roomGridY);

	GScene(const string& sceneName, const vector<MapEntry>& maps);

    virtual ~GScene();
    virtual bool init();
    virtual void update(float dt);
	virtual void onExit();
	virtual GScene* getReplacementScene();
	GSpace* getSpace();

	inline string getCurrentLevel() const { return sceneName; }
	//Return the ID of the next level, if applicable. This is used to transition
	//to next stage, and also to apply availibility unlock to that stage.
	inline virtual string getNextLevel() const { return ""; }
	inline virtual MapFragmentsList getMapFragmentsList() const { return MapFragmentsList(); }

    void setPaused(bool p);
	inline virtual void enterPause() {}
	inline virtual void exitPause() {}

	inline virtual Color4F getDefaultAmbientLight() const { return defaultAmbientLight; }

	void createDialog(const string& res, bool autoAdvance);
	void createDialog(const string& res, bool autoAdvance, zero_arity_function f);
	void stopDialog();
	Vec2 dialogPosition();
	bool isDialogActive();

	void processAdditions();

	void addActions(const vector<zero_arity_function>& _actions);
	void runActions();

	void setColorFilter(const Color4F& color);

    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
	SpaceVect getMapSize();
	SpaceRect getCameraArea();
	const vector<SpaceRect>& getMapAreas();
	const vector<bool>& getMapAreasVisited();
	const vector<bool>& getAreasVisibleOnMap();
	int getMapLocation(SpaceRect r);
	bool isInCameraArea(SpaceRect r);
	void eraseTile(int mapID, IntVec2 pos, string layer);

	inline bool isMultiMap() const { return tilemaps.size() > 1; }
	void updateMapVisibility(int roomID);
	void updateRoomsVisited(int roomID);
	void updateMultimapVisibility(SpaceVect playerPos);
	void unlockAllRooms();
	void setRoomDiscovered(size_t idx);
	void setRoomsDiscovered(rooms_bitmask rooms);
	void setRoomVisible(size_t idx);
	void setRoomsVisible(rooms_bitmask rooms);
	
	void teleportToDoor(string name);

	Layer* getLayer(sceneLayers layer);
	inline Layer* getSpaceLayer() { return getLayer(sceneLayers::space); }

	void pushMenu(MenuLayer* layer);
	void popMenu();
	void popMenuIfNonroot();

    util::multifunction<void(void)> multiInit;

	unique_ptr<ControlListener> control_listener;
	unique_ptr<graphics_context> graphicsContext;
protected:
	//Run at init time. It will call the following load methods.
	void loadMaps();
	void loadMap(const MapEntry& mapEntry);

	void loadObjectGroup(TMXObjectGroup* group, IntVec2 offset);
	void loadDynamicLoadObjects(const TMXTiledMap& map, IntVec2 offset);
	void loadMapObjects(const TMXTiledMap& map, IntVec2 offset);
	//equivalent, except load "floor" layer
	void loadFloorSegments(const TMXTiledMap& map, IntVec2 offset);
	void loadSensors(const TMXTiledMap& map, IntVec2 offset);

	//objects in this layer are Wall type by default
	void loadWalls(const TMXTiledMap& map, IntVec2 offset);
	//obvjects in this layer are Spawners
	void loadSpawners(const TMXTiledMap& map, IntVec2 offset);

	//data that is loaded to GSpace for AI use
	void loadPaths(const TMXTiledMap& map, IntVec2 offset);
	void loadWaypoints(const TMXTiledMap& map, IntVec2 offset);
	void loadAreas(const TMXTiledMap& map, IntVec2 offset);

	void loadSubrooms(const TMXTiledMap& map, IntVec2 offset);
	void loadLights(const TMXTiledMap& map, IntVec2 offset);
	//For a multi-map scene; a room sensor is created according to the bounds
	//of the map, initialized using the map's properties.
	void loadRoomFromMap(const SpaceRect& mapBounds, int roomID, const ValueMap& properties);
	//For a single map scene; room sensors are creatd from the "rooms" map
	//object layer, initialized from each map object's properties.
	void loadRoomsLayer(const TMXTiledMap& map);

	void spaceUpdateMain();

	void renderSpace();
	void redrawLightmap();
	void cycleDisplayMode();
	RenderTexture* initRenderTexture(sceneLayers sceneLayer, BlendFunc blend);
	RenderTexture* initRenderTexture(sceneLayers sceneLayer);

	void installLuaShell();
	void checkPendingScript();
	void runScriptInit();
	void runScriptUpdate();

	void waitForSpaceThread();
	void logPerformance();

	//Make sure to use a cocos map so cocos refcounting works.
	cocos2d::Map<int, Layer*> layers;
	RenderTexture* spaceRender = nullptr;
	GSpace* gspace;
	//the scale applied to the space layer
	float spaceZoom = 1;
	unique_ptr<thread> spaceUpdateThread;
	mutex spaceUpdateConditionMutex;
	condition_variable spaceUpdateCondition;

	atomic_bool spaceUpdateToRun;
	atomic_bool isPaused;
	atomic_bool isExit;

	RenderTexture* colorFilterRender = nullptr;
	DrawNode* colorFilterDraw = nullptr;

	displayMode display = displayMode::combined;
	RenderTexture* lightmapRender = nullptr;
	DrawNode* lightmapDrawNode = nullptr;
	DrawNode* lightmapBackground = nullptr;

	vector<zero_arity_function> actionsToRun;
	mutex actionsMutex;

	vector<MenuLayer*> menuStack;
	Dialog* dialog = nullptr;

	string sceneName;
	IntVec2 dimensions;

	Vector<TMXTiledMap*> tilemaps;
	vector<MapEntry> maps;
	vector<SpaceRect> mapAreas;
	vector<bool> mapAreasVisited;
	vector<bool> mapAreasVisibleOnMap;
	SpaceRect cameraArea;
	int crntMap = -1;

	unique_ptr<Lua::Inst> ctx;
	//The shell that is installed in the current scene.
	LuaShell* luaShell;
	string pendingScript;
};

#endif /* GScene_hpp */
