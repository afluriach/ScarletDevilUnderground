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
    enum EventPriorities
    {
        KeyGlobalListenerEvent = 1,
        KeyRegisterEvent,
    };
    
    static const int width = 800;
    static const int height = 500;
    
    static const int framesPerSecond = 60;
    constexpr static float secondsPerFrame = 1.0 / framesPerSecond;
    
    static const int pixelsPerTile = 128;
    constexpr static float tilesPerPixel = 1.0/pixelsPerTile;
    
    static const bool showStats = true;
    static const bool logSprites = false;
    static const string title;
    
    static inline Vec2 getScreenCenter()
    {
        return Vec2(width/2, height/2);
    }
    
    App();
    virtual ~App();

    virtual void initGLContextAttrs();

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
    
    
    KeyRegister* keyRegister;
    Lua::Inst lua;
protected:
    KeyListener keyListener;
    //The shell that is installed in the current scene.
    LuaShell* luaShell;
    string pendingScript;
    
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

