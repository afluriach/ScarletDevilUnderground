//
//  scenes.h
//  FlansBasement
//
//  Created by Toni on 11/23/15.
//
//

#ifndef scenes_h
#define scenes_h

class GScene : public Layer
{
public:
    //The order of initialization events, as performed by GScene and it's various dervied classes.
    enum initOrder{
        //This includes all top-level GScene init, as well running Layer::init and Node::scheduleUpdate.
        core = 1,
        //Loading map objects if applicable
        mapLoad,
        //Running GSpace::loadAdditions, if applicable
        loadObjects,
        //Objects that wish to query the GSpace, including looking up other objects that are expected
        //to be loaded.
        postLoadObjects,
    };

    typedef function<void () > AdapterType;
    //Map each class name to a constructor adapter function.
    static const map<string,AdapterType> adapters;
    
    static GScene* crntScene;

    static inline void runScene(const string& name)
    {
        auto it = adapters.find(name);
        
        if(it == adapters.end()){
            throw runtime_error("runScene: " + name + " not found.");
        }
        else
        {
            it->second();
        }
    }
    
    //Get gspace if this scene is a space scene, null otherwise.
    static GSpace* getSpace();

//Rather than managing overrides to the init method, a scene simply registers their own.
//Init methods must be installed at construction time.

    inline GScene()
    {
        //Updater has to be scheduled at init time.
        multiInit.insertWithOrder(bind(&GScene::initUpdate,this), initOrder::core);
    
        crntScene = this;
    }

    inline virtual bool init()
    {
        Layer::init();
        
        multiInit();
        
        return true;
    }
    
    inline virtual void update(float dt)
    {
        multiUpdate(dt);
    }
    
    void move(const Vec2& v);
    //The different vector type is intentional, as Chipmunk vector implies
    //unit space as opposed to pixel space.
    void setUnitPosition(const SpaceVect& v);
    
    util::multifunction<void()> multiInit;
    util::multifunction<void(float)> multiUpdate;
private:
    inline void initUpdate()
    {
        scheduleUpdate();
    }
};

class GSpaceScene : virtual public GScene
{
public:
    inline GSpaceScene() : gspace(this)
    {
        multiInit.insertWithOrder(bind(&GSpaceScene::processAdditions, this), initOrder::loadObjects);
        multiUpdate += bindMethod(&GSpaceScene::updateSpace,this);
    }

    GSpace gspace;
    
    inline void updateSpace(float dt)
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
    inline MapScene(const string& res) : mapRes(res)
    {
        multiInit.insertWithOrder(bind(&MapScene::loadMap, this),initOrder::mapLoad);
    }
protected:
    string mapRes;
    Layer* mapLayer;
    TMXTiledMap* tileMap;
    
    void loadMapObjects(const TMXTiledMap& map);
    //Add a map object layer to space.
    void loadObjectGroup(TMXObjectGroup* group);
    
    //Run at init time.
    void loadMap();
};


#endif /* scenes_h */
