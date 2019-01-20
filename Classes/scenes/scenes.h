//
//  scenes.h
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#ifndef scenes_h
#define scenes_h

#include "Graphics.h"
#include "multifunction.h"
#include "types.h"

class ControlListener;
class Dialog;
class GSpace;
class HUD;
class LuaShell;
class PatchConAnimation;
class PlayScene;
class TimedLoopAnimation;

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
		renderSpace,
        hudUpdate,
    };
    
    enum class sceneLayers{
        //Represents the world and objects in it (map and all gspace objects).
        //GraphicsLayer is used for z-ordering inside of this layer.
        //It is the only layer that moves with the camera
		begin = 1,
        space = 1,
		lightmapBackground,
		lightmap,
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

	void processAdditions();

	void addAction(function<void(void)>, updateOrder order);
	void addAction(pair<function<void(void)>, updateOrder> entry);

	void runActionsWithOrder(updateOrder order);
	void addSpriteActions(const vector<function<void()>>& v);

	unsigned int addLightSource(CircleLightArea light);
	unsigned int addLightSource(AmbientLightArea light);
	unsigned int addLightSource(ConeLightArea light);
	void updateLightSource(unsigned int id, ConeLightArea light);
	void removeLightSource(unsigned int id);
	void setLightSourcePosition(unsigned int id, SpaceVect pos);

	unsigned int createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	unsigned int createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	unsigned int createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	unsigned int createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom);

	void loadAgentAnimation(unsigned int id, string path, bool isAgentAnimation);
	void setAgentAnimationDirection(unsigned int id, Direction d);
	void setAgentAnimationFrame(unsigned int id, int frame);

	void clearDrawNode(unsigned int id);
	void drawSolidRect(unsigned int id, Vec2 lowerLeft, Vec2 upperRight, Color4F color);
	void drawSolidCone(unsigned int id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color);
	void drawSolidCircle(unsigned int id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color);

	void runSpriteAction(unsigned int id, ActionGeneratorType generator);
	void stopSpriteAction(unsigned int id, cocos_action_tag action);
	void stopAllSpriteActions(unsigned int id);
	void removeSprite(unsigned int id);
	void removeSpriteWithAnimation(unsigned int id, ActionGeneratorType generator);
	void setSpriteVisible(unsigned int id, bool val);
	void setSpriteOpacity(unsigned int id, unsigned char op);
	void setSpriteTexture(unsigned int id, string path);
	void setSpriteAngle(unsigned int id, float cocosAngle);
	void setSpritePosition(unsigned int id, Vec2 pos);
	void setSpriteZoom(unsigned int id, float zoom);

    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
	SpaceVect getMapSize();
	CCRect getCameraArea();
	const vector<CCRect>& getMapAreas();
	const vector<bool>& getMapAreasVisited();
	int getMapLocation(CCRect r);
	bool isInCameraArea(CCRect r);
	bool isInPlayerRoom(SpaceVect v);
	int getPlayerRoom();

	Layer* getLayer(sceneLayers layer);
	inline Layer* getSpaceLayer() { return getLayer(sceneLayers::space); }

    util::multifunction<void(void)> multiInit;
    util::multifunction<void(void)> multiUpdate;

	unique_ptr<ControlListener> control_listener;
protected:
	static unsigned int nextLightID;

	//Run at init time. It will call the following load methods.
	void loadMaps();
	void loadMap(const MapEntry& mapEntry);

	void loadMapObjects(const TMXTiledMap& map, IntVec2 offset);
	//Add a map object layer to space.
	void loadPaths(const TMXTiledMap& map, IntVec2 offset);
	void loadWaypoints(const TMXTiledMap& map, IntVec2 offset);
	void loadRooms(const TMXTiledMap& map, IntVec2 offset);
	void loadFloorSegments(const TMXTiledMap& map, IntVec2 offset);
	void loadObjectGroup(TMXObjectGroup* group, IntVec2 offset);
	void loadWalls(const TMXTiledMap& map, IntVec2 offset);

	void initEnemyStats();
	void spaceUpdateMain();

	void updateMapVisibility();
	void renderSpace();
	void redrawLightmap();
	void cycleDisplayMode();

	void installLuaShell();
	void checkPendingScript();
	void runScriptInit();
	void runScriptUpdate();

	Node* getSpriteAsNode(unsigned int id);
	void _removeSprite(unsigned int id);

	//Make sure to use a cocos map so cocos refcounting works.
	cocos2d::Map<int, Layer*> layers;
	RenderTexture* spaceRender = nullptr;
	Vector<TMXTiledMap*> tilemaps;
	GSpace* gspace;
	//the scale applied to the space layer
	float spaceZoom = 1;
	unique_ptr<thread> spaceUpdateThread;
	atomic_int spaceUpdatesToRun;

	unsigned int nextSpriteID = 1;
	map<unsigned int, Node*> crntSprites;
	map<unsigned int, DrawNode*> drawNodes;
	map<unsigned int, TimedLoopAnimation*> animationSprites;
	map<unsigned int, PatchConAnimation*> agentSprites;
	vector<function<void()>> spriteActions;
	mutex spriteActionsMutex;
	mutex lightmapMutex;

	displayMode display = displayMode::combined;
	Color4F ambientLight = Color4F::WHITE;
	RenderTexture* lightmapRender = nullptr;
	DrawNode* lightmapDrawNode = nullptr;
	DrawNode* lightmapBackground = nullptr;
	map<unsigned int, AmbientLightArea> ambientLights;
	map<unsigned int, CircleLightArea> circleLights;
	map<unsigned int, ConeLightArea> coneLights;
	map<unsigned int, RadialGradient*> lightmapRadials;

	list<pair<function<void(void)>, updateOrder>> actions;
	mutex actionsMutex;

	Dialog* dialog = nullptr;

	string sceneName;
	IntVec2 dimensions;
	vector<MapEntry> maps;
	vector<CCRect> mapAreas;
	vector<bool> mapAreasVisited;
	CCRect cameraArea;
	int crntMap = -1;

	unique_ptr<Lua::Inst> ctx;
	//The shell that is installed in the current scene.
	LuaShell* luaShell;
	string pendingScript;

	bool isPaused = false;
	bool isExit = false;
};

#endif /* scenes_h */
