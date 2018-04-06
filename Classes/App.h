#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by Director.
*/

class GScene;

class  App : private Application
{
public:
    friend class GScene;

    enum class EventPriorities
    {
        KeyGlobalListenerEvent = 1,
        KeyRegisterEvent,
    };
    
    static const int width = 1600;
    static const int height = 1000;
    
    float screenscale = 1;
    float dpiscale = 1;
    
    static const int framesPerSecond = 60;
    constexpr static float secondsPerFrame = 1.0f / framesPerSecond;
    
    constexpr static float Gaccel = 9.8f;
    
    static const int pixelsPerTile = 128;
    constexpr static float tilesPerPixel = 1.0f/pixelsPerTile;
    constexpr static float viewWidth = 12.0f;
    
    static const bool useRepl = false;
    static const bool showStats = true;
    static const bool logSprites = false;
    static const string title;
    
    static const vector<string> shaderFiles;
    
    static inline Vec2 getScreenCenter()
    {
        return Vec2(width/2, height/2);
    }
    
    App();
    virtual ~App();

    virtual void initGLContextAttrs();
    
    void loadShaders();

    /**
    @brief    Implement Director and Scene init code here.
    @return true    Initialize success, app continue.
    @return false   Initialize failed, app terminate.
    */
    virtual bool applicationDidFinishLaunching();

    /**
    @brief  The function be called when the application enter background
    @param  the pointer of the application
    */
    virtual void applicationDidEnterBackground();

    /**
    @brief  The function be called when the application enter foreground
    @param  the pointer of the application
    */
    virtual void applicationWillEnterForeground();
    
    void end();
    
    //Methods for controlling the active scene; wraps calls to Director.
    
    template <typename T>
    inline void pushScene()
    {
        Director::getInstance()->pushScene(createSceneFromLayer<T>());
    }

    inline void popScene()
    {
        Director::getInstance()->popScene();
    }

    template <typename T>
    inline void runScene()
    {
        Director::getInstance()->runScene(createSceneFromLayer<T>());
    }
    
    //Generate [min,max)
    inline float getRandomFloat(float min, float max){
        float u01 = randomFloat(randomEngine);
        
        return (min + u01*(max-min));
    }
    
    //Generate [min,max]
    inline int getRandomInt(int min, int max){
        return randomInt(randomEngine, boost::random::uniform_int_distribution<int>::param_type(min,max));
    }
    
    boost::random::uniform_01<float> randomFloat;
    boost::random::uniform_int_distribution<int> randomInt;
    boost::random::mt19937 randomEngine;
    
    KeyRegister* keyRegister;
    Lua::Inst lua;
    Lua::Inst replInst;
protected:
    KeyListener keyListener;
    //The shell that is installed in the current scene.
    LuaShell* luaShell;
    string pendingScript;
    thread* luaReplThread;
    
    Scene* createSceneFromLayer(Layer* layer);

    template <typename T>
    inline Scene* createSceneFromLayer()
    {
        return createSceneFromLayer(T::create());
    }
    
    void installLuaShell(GScene* scene);
    void checkPendingScript();
};

extern App* app;

#endif // _APP_DELEGATE_H_

