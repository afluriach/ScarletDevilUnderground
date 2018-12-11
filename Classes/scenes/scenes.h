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

class Dialog;
class GSpace;
class HUD;
class ControlListener;
class PlayScene;

namespace Lua{
    class Inst;
}

class GScene : public Layer
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

    typedef function<void () > AdapterType;
	typedef pair<string, IntVec2> MapEntry;

    //Map each class name to a constructor adapter function.
	//Defined in SceneMapping.cpp.
    static const unordered_map<string,AdapterType> adapters;

	static const int dialogEdgeMargin;

    static GScene* crntScene;
	static string crntSceneName;
	static string crntReplayName;

	static void runScene(const string& name);
	static void runSceneWithReplay(const string& sceneName, const string& replayName);
	static void restartScene();
	static void restartReplayScene();

	vector<MapEntry> singleMapEntry(const string& mapName);

	GScene(const string& mapName);
	GScene(const string& sceneName, const vector<MapEntry>& maps);

    virtual ~GScene();
    bool init();
    void update(float dt);
    
    void setPaused(bool p);
	inline virtual void enterPause() {}
	inline virtual void exitPause() {}

	void createDialog(const string& res, bool autoAdvance);
	void stopDialog();
	Vec2 dialogPosition();

	void updateSpace();
	void processAdditions();

    void move(const Vec2& v);
    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
	SpaceVect getMapSize();

	Layer* getLayer(sceneLayers layer);

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

	void runScriptInit();
	void runScriptUpdate();

	//Make sure to use a cocos map so cocos refcounting works.
	cocos2d::Map<int, Layer*> layers;
	GSpace* gspace;
	//the scale applied to the space layer
	float spaceZoom = 1;

	vector<MapEntry> maps;
	IntVec2 dimensions;

	unique_ptr<Lua::Inst> ctx;

	bool isPaused = false;
};

#endif /* scenes_h */
