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
        multiInit += bind(&GScene::initUpdate,this);
        
        crntScene = this;
    }

    inline virtual bool init()
    {
        cocos2d::Layer::init();
        
        multiInit();
        
        return true;
    }
    
    inline virtual void update(float dt)
    {
        multiUpdate(dt);
    }
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
        multiUpdate += bind(&GSpaceScene::updateSpace,this, placeholders::_1);
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
        multiInit += bind(&MapScene::loadMap, this);
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
