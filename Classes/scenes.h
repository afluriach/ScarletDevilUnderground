//
//  scenes.h
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#ifndef scenes_h
#define scenes_h

class PlayScene;
class HUD;

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
        //Objects that wish to query the GSpace, including looking up other objects that are expected
        //to be loaded.
        postLoadObjects,
    };
    
    enum class updateOrder{
        //Update tick on GSpace and all objects, if applicable
        runShellScript=1,
        spaceUpdate,
        //General scene update logic
        sceneUpdate,
        moveCamera,
        hudUpdate,
    };
    
    enum class sceneLayers{
        //Represents the world and objects in it (map and all gspace objects).
        //GraphicsLayer is used for z-ordering inside of this layer.
        //It is the only layer that moves with the camera
        space = 1,
        dialogBackground,
        dialog,
        hud,
        menu,
        luaShell,
        end
    };

    const int dialogEdgeMargin = 30;

    typedef function<void () > AdapterType;
    //Map each class name to a constructor adapter function.
    static const unordered_map<string,AdapterType> adapters;
    
    static GScene* crntScene;
	static string crntSceneName;
	KeyListener keyListener;

    static void runScene(const string& name);
	static void restartScene();

//Helpers to get the current scene as a specific sub-type (if applicable)
    static PlayScene* playScene();

//Getters for fields associated with specific types of scenes.
    //Get gspace if this scene is a space scene, null otherwise.
    static GSpace* getSpace();
    //get HUD if this is a PlayScene
    static HUD* getHUD();

//Rather than managing overrides to the init method, a scene simply registers their own.
//Init methods must be installed at construction time.

    GScene();
    bool init();
    void update(float dt);
    
    void setPaused(bool p);
    
    void move(const Vec2& v);
    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
    
    util::multifunction<void(void)> multiInit;
    util::multifunction<void(void)> multiUpdate;
    
    //Wrapper to call a method of a derived type with a GScene this.
    //template<typename Derived, void (Derived::*Method)(void)>
    //function<void(GScene*)> wrap()
    //{
    //    return wrapAsBaseMethod<GScene, Derived, Method>();
    //}
    
    inline Layer* getLayer(sceneLayers layer){
        auto it = layers.find(static_cast<int>(layer));
        if(it == layers.end()) return nullptr;
        return it->second;
    }
    
    //Nodes should not be directly added to a GScene, but rather use one of the defined layers.
    inline void addChild(Node* n, int z) {throw runtime_error("addChild: Node should not be added directly to GScene.");}
    inline void addChild(Node* n) {throw runtime_error("addChild: Node should not be added directly to GScene.");};
    
    void createDialog(const string& res, bool autoAdvance);
    void stopDialog();
    inline Vec2 dialogPosition()
    {
        return Vec2(App::width/2, Dialog::height/2 + dialogEdgeMargin);
    }
    
    static inline bool isDialogActive(){
        return crntScene->dialogNode != nullptr;
    }
    
protected:
    Dialog* dialogNode = nullptr;
    
    bool isPaused = false;

    //Make sure to use a cocos map so cocos refcounting works.
    cocos2d::Map<int,Layer*> layers;
    //the scale applied to the space layer
    float spaceZoom = 1;
    inline void initUpdate()
    {
        scheduleUpdate();
    }
};

class GSpaceScene : virtual public GScene
{
public:
    inline GSpaceScene() : gspace(getLayer(sceneLayers::space))
    {
        multiInit.insertWithOrder(
            wrap_method(GSpaceScene,processAdditions,this),
            static_cast<int>(initOrder::loadObjects)
        );
        multiUpdate.insertWithOrder(
            wrap_method(GSpaceScene,updateSpace,this),
            static_cast<int>(updateOrder::spaceUpdate)
        );
    }

    GSpace gspace;
    
    inline void updateSpace()
    {
        gspace.update();
    }
    
    //Process added objects so they are availible to anything that queries the GSpace.
    //This is needed for any scene init code that wishes to access loaded objects.
    inline void processAdditions(){
        gspace.processAdditions();
    }
};

class MapScene : virtual public GSpaceScene
{
public:
    inline MapScene(const string& res) : mapRes("maps/"+res+".tmx")
    {
        multiInit.insertWithOrder(
            wrap_method(MapScene,loadMap,this),
            static_cast<int>(initOrder::mapLoad)
        );
    }
    inline SpaceVect getMapSize(){
        return toChipmunk(tileMap->getMapSize());
    }

protected:
    string mapRes;
    Layer* mapLayer;
    TMXTiledMap* tileMap;
    
    void loadMapObjects(const TMXTiledMap& map);
    //Add a map object layer to space.
	void loadPaths(const TMXTiledMap& map);
    void loadObjectGroup(TMXObjectGroup* group);
    void loadWalls();
    
    //Run at init time.
    void loadMap();
};

class ScriptedScene : virtual public GScene
{
public:
    inline ScriptedScene(const string& res) :
    ctx("scene")
    {
        multiInit.insertWithOrder(
            wrap_method(ScriptedScene, runInit,this),
            static_cast<int>(initOrder::postLoadObjects)
        );
        multiUpdate.insertWithOrder(
            wrap_method(ScriptedScene,runUpdate,this),
            static_cast<int>(updateOrder::sceneUpdate)
        );
        
        if(!res.empty())
        {
            string path = "scripts/scenes/"+res+".lua";
        
            if(!FileUtils::getInstance()->isFileExist(path))
                log("ScriptedScene: %s script does not exist.", res.c_str());
            else
                ctx.runFile(path);
        }
    }

    void runInit();
    void runUpdate();
    
    Lua::Inst ctx;
};

#endif /* scenes_h */
