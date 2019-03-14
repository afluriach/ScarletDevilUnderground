//
//  GScene.hpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#ifndef GScene_hpp
#define GScene_hpp

#include "graphics_types.h"
#include "multifunction.h"

class AgentBodyShader;
class ConeShader;
class ControlListener;
class Dialog;
class GSpace;
class HUD;
class LuaShell;
class MenuLayer;
class PatchConAnimation;
class PlayScene;
class RadialGradient;
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
	static void runSceneWithReplay(const string& replayName);
	static void restartScene();
	static void restartReplayScene();

	vector<MapEntry> singleMapEntry(const string& mapName);
	static const IntVec2 getRoomOffset(IntVec2 roomSize, int roomGridX, int roomGridY);

	GScene(const string& sceneName, const vector<MapEntry>& maps);

    virtual ~GScene();
    virtual bool init();
    void update(float dt);
	virtual void onExit();
	virtual GScene* getReplacementScene();
	GSpace* getSpace();

    void setPaused(bool p);
	inline virtual void enterPause() {}
	inline virtual void exitPause() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(0.5f, 0.5f, 0.5f,1.0f); }

	void createDialog(const string& res, bool autoAdvance);
	void createDialog(const string& res, bool autoAdvance, zero_arity_function f);
	void stopDialog();
	Vec2 dialogPosition();
	bool isDialogActive();

	void processAdditions();

	void addActions(const vector<pair<zero_arity_function, SceneUpdateOrder>>& _actions);

	void runActionsWithOrder(SceneUpdateOrder order);

	void setColorFilter(const Color4F& color);

	SpriteID getSpriteID();
	LightID getLightID();

	void addLightSource(LightID id, CircleLightArea light);
	void addLightSource(LightID id, AmbientLightArea light);
	void addLightSource(LightID id, ConeLightArea light);
	void addLightSource(LightID id, SpriteLightArea light);
	void removeLightSource(LightID id);
	void setLightSourcePosition(LightID id, SpaceVect pos);
	void setLightSourceAngle(LightID id, SpaceFloat a);
	void setLightSourceColor(LightID id, Color4F color);

	void createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createAgentSprite(SpriteID id, string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom);
	void createDamageIndicator(float val, SpaceVect pos);
	void createAgentBodyShader(
		SpriteID id, GraphicsLayer layer,
		Color4F bodyColor, Color4F coneColor,
		float bodyRadius, float coneRadius,
		float thickness, Vec2 position,
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
	void eraseTile(int mapID, IntVec2 pos, string layer);

	inline bool isMultiMap() const { return tilemaps.size() > 1; }
	void updateMapVisibility(SpaceVect playerPos);
	void updateMultimapVisibility(SpaceVect playerPos);
	void updateRoomsVisited(SpaceVect playerPos);
	void unlockAllRooms();
	
	void teleportToDoor(string name);

	Layer* getLayer(sceneLayers layer);
	inline Layer* getSpaceLayer() { return getLayer(sceneLayers::space); }

	void pushMenu(MenuLayer* layer);
	void popMenu();
	void popMenuIfNonroot();

    util::multifunction<void(void)> multiInit;
    util::multifunction<void(void)> multiUpdate;

	unique_ptr<ControlListener> control_listener;
protected:
	atomic_uint nextLightID = 1;

	//Run at init time. It will call the following load methods.
	void loadMaps();
	void loadMap(const MapEntry& mapEntry);

	void loadObjectGroup(TMXObjectGroup* group, IntVec2 offset);
	void loadMapObjects(const TMXTiledMap& map, IntVec2 offset);
	//equivalent, except load "floor" layer
	void loadFloorSegments(const TMXTiledMap& map, IntVec2 offset);
	void loadSensors(const TMXTiledMap& map, IntVec2 offset);

	//objects in this layer are Wall type by default
	void loadWalls(const TMXTiledMap& map, IntVec2 offset);

	//data that is loaded to GSpace for AI use
	void loadPaths(const TMXTiledMap& map, IntVec2 offset);
	void loadWaypoints(const TMXTiledMap& map, IntVec2 offset);

	void loadSubrooms(const TMXTiledMap& map, IntVec2 offset);
	void loadLights(const TMXTiledMap& map, IntVec2 offset);
	//For a multi-map scene; a room sensor is created according to the bounds
	//of the map, initialized using the map's properties.
	void loadRoomFromMap(const SpaceRect& mapBounds, int roomID, const ValueMap& properties);
	//For a single map scene; room sensors are creatd from the "rooms" map
	//object layer, initialized from each map object's properties.
	void loadRoomsLayer(const TMXTiledMap& map);

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

	void queueActions();
	void waitForSpaceThread();
	void logPerformance();

	Node* getSpriteAsNode(SpriteID id);
	void _removeSprite(SpriteID id);

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

	atomic_uint nextSpriteID = 1;
	unordered_map<SpriteID, Node*> graphicsNodes;
	unordered_map<SpriteID, Sprite*> crntSprites;
	unordered_map<SpriteID, DrawNode*> drawNodes;
	unordered_map<SpriteID, TimedLoopAnimation*> animationSprites;
	unordered_map<SpriteID, PatchConAnimation*> agentSprites;
	unordered_map<SpriteID, AgentBodyShader*> agentShaders;

	displayMode display = displayMode::combined;
	RenderTexture* lightmapRender = nullptr;
	DrawNode* lightmapDrawNode = nullptr;
	DrawNode* lightmapBackground = nullptr;
	unordered_map<LightID, Node*> lightmapNodes;

	unordered_map<SceneUpdateOrder, vector<zero_arity_function>> actions;
	vector<pair<zero_arity_function, SceneUpdateOrder>> actionsToAdd;
	mutex actionsMutex;

	vector<MenuLayer*> menuStack;
	Dialog* dialog = nullptr;

	string sceneName;
	IntVec2 dimensions;

	vector<DrawNode*> roomMasks;
	Vector<TMXTiledMap*> tilemaps;
	vector<MapEntry> maps;
	vector<SpaceRect> mapAreas;
	vector<bool> mapAreasVisited;
	SpaceRect cameraArea;
	int crntMap = -1;

	unique_ptr<Lua::Inst> ctx;
	//The shell that is installed in the current scene.
	LuaShell* luaShell;
	string pendingScript;
};

#endif /* GScene_hpp */
