//
//  scenes.h
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#ifndef scenes_h
#define scenes_h

#include "multifunction.h"
#include "types.h"

class ControlListener;
class Dialog;
class GSpace;
class HUD;
class LuaShell;
class PlayScene;
class SpaceLayer;

namespace Lua{
    class Inst;
}

class GScene : public Scene
{
public:
    //The order of initialization events, as performed by GScene and it's various dervied classes.
    enum class initOrder{
        //This includes all top-level GScene init, as well running Layer::init and Node::scheduleUpdate.
        core = 1,
        //Loading map objects if applicable
        mapLoad,
        //Running GSpace::loadAdditions, if applicable
        loadObjects,
        initHUD,
		postInitHUD,
		initRoomMask,
        //Objects that wish to query the GSpace, including looking up other objects that are expected
        //to be loaded.
        postLoadObjects,
    };
    
    enum class updateOrder{
        //Update tick on GSpace and all objects, if applicable
		updateControls=1,
		runShellScript,
        spaceUpdate,
        //General scene update logic
        sceneUpdate,
        moveCamera,
        hudUpdate,
		roomMaskUpdate,
    };
    
    enum class sceneLayers{
        //Represents the world and objects in it (map and all gspace objects).
        //GraphicsLayer is used for z-ordering inside of this layer.
        //It is the only layer that moves with the camera
        space = 1,
		roomMask,
        dialogBackground,
        dialog,
        hud,
        menu,
        luaShell,
        end
    };

    typedef function<GScene* () > AdapterType;
	typedef pair<string, IntVec2> MapEntry;

    //Map each class name to a constructor adapter function.
	//Defined in SceneMapping.cpp.
    static const unordered_map<string,AdapterType> adapters;

	static const int dialogEdgeMargin;
	static const bool scriptLog;

	static string crntSceneName;
	static string crntReplayName;
	static bool suppressGameOver;

	static GScene* runScene(const string& name);
	static void runSceneWithReplay(const string& sceneName, const string& replayName);
	static void restartScene();
	static void restartReplayScene();

	vector<MapEntry> singleMapEntry(const string& mapName);
	static const IntVec2 getRoomOffset(IntVec2 roomSize, int roomGridX, int roomGridY);

	GScene(const string& sceneName, const vector<MapEntry>& maps);

    virtual ~GScene();
    virtual bool init();
    void update(float dt);
	virtual GScene* getReplacementScene();
	GSpace* getSpace();

    void setPaused(bool p);
	inline virtual void enterPause() {}
	inline virtual void exitPause() {}

	void createDialog(const string& res, bool autoAdvance);
	void stopDialog();
	Vec2 dialogPosition();
	bool isDialogActive();

	void updateSpace();
	void processAdditions();

	void addAction(function<void(void)>, updateOrder order);
	void addAction(pair<function<void(void)>, updateOrder> entry);

    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
	SpaceVect getMapSize();
	CCRect getCameraArea();
	bool isInCameraArea(CCRect r);
	bool isInPlayerRoom(SpaceVect v);

	Layer* getLayer(sceneLayers layer);
	inline SpaceLayer* getSpaceLayer() const { return spaceLayer; }

    util::multifunction<void(void)> multiInit;
    util::multifunction<void(void)> multiUpdate;

	unique_ptr<ControlListener> control_listener;
protected:
	//Run at init time. It will call the following load methods.
	void loadMaps();
	void loadMap(const MapEntry& mapEntry);

	void loadMapObjects(const TMXTiledMap& map, IntVec2 offset);
	//Add a map object layer to space.
	void loadPaths(const TMXTiledMap& map, IntVec2 offset);
	void loadRooms(const TMXTiledMap& map, IntVec2 offset);
	void loadFloorSegments(const TMXTiledMap& map, IntVec2 offset);
	void loadObjectGroup(TMXObjectGroup* group, IntVec2 offset);
	void loadWalls(const TMXTiledMap& map, IntVec2 offset);

	void updateMapVisibility();

	void installLuaShell();
	void checkPendingScript();
	void runScriptInit();
	void runScriptUpdate();

	void runActionsWithOrder(updateOrder order);

	//Make sure to use a cocos map so cocos refcounting works.
	cocos2d::Map<int, Layer*> layers;
	SpaceLayer* spaceLayer = nullptr;
	Vector<TMXTiledMap*> tilemaps;
	GSpace* gspace;
	//the scale applied to the space layer
	float spaceZoom = 1;

	list<pair<function<void(void)>, updateOrder>> actions;

	Dialog* dialog = nullptr;

	string sceneName;
	IntVec2 dimensions;
	vector<MapEntry> maps;
	vector<CCRect> mapAreas;
	CCRect cameraArea;
	int crntMap = -1;

	unique_ptr<Lua::Inst> ctx;
	//The shell that is installed in the current scene.
	LuaShell* luaShell;
	string pendingScript;

	bool isPaused = false;
};

#endif /* scenes_h */
