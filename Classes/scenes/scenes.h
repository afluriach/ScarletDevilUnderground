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

	void setColorFilter(const Color4F& color);

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

    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
	SpaceVect getMapSize();
	SpaceRect getCameraArea();
	const vector<SpaceRect>& getMapAreas();
	const vector<bool>& getMapAreasVisited();
	int getMapLocation(SpaceRect r);
	bool isInCameraArea(SpaceRect r);
	bool isInPlayerRoom(SpaceVect v);
	int getPlayerRoom();

	void updateMapVisibility(SpaceVect playerPos);
	
	void teleportToDoor(string name);

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
	void loadFloorSegments(const TMXTiledMap& map, IntVec2 offset);
	void loadObjectGroup(TMXObjectGroup* group, IntVec2 offset);
	void loadWalls(const TMXTiledMap& map, IntVec2 offset);

	void initEnemyStats();
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

	Node* getSpriteAsNode(SpriteID id);
	void _removeSprite(SpriteID id);

	//Make sure to use a cocos map so cocos refcounting works.
	cocos2d::Map<int, Layer*> layers;
	RenderTexture* spaceRender = nullptr;
	Vector<TMXTiledMap*> tilemaps;
	GSpace* gspace;
	//the scale applied to the space layer
	float spaceZoom = 1;
	unique_ptr<thread> spaceUpdateThread;
	atomic_int spaceUpdatesToRun;

	RenderTexture* colorFilterRender = nullptr;
	DrawNode* colorFilterDraw = nullptr;

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
	vector<SpaceRect> mapAreas;
	vector<bool> mapAreasVisited;
	SpaceRect cameraArea;
	int crntMap = -1;

	unique_ptr<Lua::Inst> ctx;
	//The shell that is installed in the current scene.
	LuaShell* luaShell;
	string pendingScript;

	bool isPaused = false;
	bool isExit = false;
};

#endif /* scenes_h */
