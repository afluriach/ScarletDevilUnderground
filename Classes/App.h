#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

#include "cocos2d.h"

#include "controls.h"
#include "LuaAPI.hpp"
#include "LuaShell.hpp"

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by Director.
*/

class  App : private cocos2d::Application
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
    static const std::string title;
    
    static inline cocos2d::Vec2 getScreenCenter()
    {
        return cocos2d::Vec2(width/2, height/2);
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
        cocos2d::Director::getInstance()->pushScene(createSceneFromLayer<T>());
    }

    inline void popScene()
    {
        cocos2d::Director::getInstance()->popScene();
    }

    template <typename T>
    inline void runScene()
    {
        cocos2d::Director::getInstance()->runWithScene(createSceneFromLayer<T>());
    }
    
    
    KeyRegister* keyRegister;
    Lua::Inst lua;
protected:
    KeyListener keyListener;
    //The shell that is installed in the current scene.
    LuaShell* luaShell;

    template <typename T>
    inline cocos2d::Scene* createSceneFromLayer()
    {
        cocos2d::Scene* scene  = cocos2d::Scene::create();
        cocos2d::Layer* layer = T::create();
        scene->addChild(layer,1);
        
        installLuaShell(scene);
        
        return scene;
    }
    void installLuaShell(cocos2d::Scene* scene);
};

extern App* app;

#endif // _APP_DELEGATE_H_

