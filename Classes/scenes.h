//
//  scenes.h
//  FlansBasement
//
//  Created by Toni on 11/23/15.
//
//

#ifndef scenes_h
#define scenes_h

using namespace std;
USING_NS_CC;

class GScene : public Layer
{
public:
    typedef std::function<void () > AdapterType;
    //Map each class name to a constructor adapter function.
    static const std::map<std::string,AdapterType> adapters;
    
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
        addInit(bind(&GScene::initUpdate,this));
        
        crntScene = this;
    }

    inline void addInit(function<void()> init)
    {
        initializers.push_back(init);
    }
    inline virtual bool init()
    {
        cocos2d::Layer::init();
        
        BOOST_FOREACH(function<void()> f, initializers)
        {
            f();
        }
        
        return true;
    }
    
    inline void addUpdate(function<void(float)> update)
    {
        updaters.push_back(update);
    }

    inline virtual void update(float dt)
    {
        BOOST_FOREACH(function<void(float)> f, updaters)
        {
            f(dt);
        }
    }
private:
    vector<function<void()>> initializers;
    vector<function<void(float)>> updaters;
    
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
        addUpdate(bind(&GSpaceScene::updateSpace,this, placeholders::_1));
    }

    GSpace gspace;
    
    inline void updateSpace(float dt)
    {
        gspace.update();
    }
};

class MapScene : virtual public GSpaceScene
{
public:
    inline MapScene(const string& res) : mapRes(res)
    {
        addInit(bind(&MapScene::loadMap, this));
    }
protected:
    string mapRes;
    cocos2d::Layer* mapLayer;
    cocos2d::TMXTiledMap* tileMap;
    
    void loadMapObjects(const cocos2d::TMXTiledMap& map);
    //Add a map object layer to space.
    void loadObjectGroup(TMXObjectGroup* group);
    
    //Run at init time.
    void loadMap();
};


#endif /* scenes_h */
