#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

#include "cocos2d.h"

#include "controls.h"

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by Director.
*/

class  App : private cocos2d::Application
{
public:
    enum EventPriorities
    {
        KeyRegisterEvent = 1
    };
    
    static const int framesPerSecond = 60;
    constexpr static float secondsPerFrame = 1.0 / framesPerSecond;
    
    static const int pixelsPerTile = 128;
    constexpr static float tilesPerPixel = 1.0/pixelsPerTile;
    
    static const bool showStats = true;
    static const std::string title;
    
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
    
    KeyRegister* keyRegister;
};

extern App* app;

#endif // _APP_DELEGATE_H_

