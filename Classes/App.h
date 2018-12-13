#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by Director.
*/

#include "LuaAPI.hpp"
#include "LuaShell.hpp"

class App;
class ControlRegister;
class Dialog;
class GScene;
class GSpace;
class HUD;
class KeyRegister;
class PlayScene;

extern App* app;

class  App : private Application
{
public:
    enum class EventPriorities
    {
        KeyGlobalListenerEvent = 1,
        KeyRegisterEvent,
    };
    
    static unsigned int width;
    static unsigned int height;

	static const unsigned int baseWidth = 1600;
    
    //Game options set through the script API
    float screenscale = 1;
    float dpiscale = 1;
    bool fullscreen = false;
    
    bool suppressGameOver = false;
    
    static const int framesPerSecond = 60;
    constexpr static float secondsPerFrame = 1.0f / framesPerSecond;
    
    constexpr static float Gaccel = 9.8f;
    
    static const int pixelsPerTile = 128;
    constexpr static float tilesPerPixel = 1.0f/pixelsPerTile;
    constexpr static float viewWidth = 22.5f;
    
    static const bool showStats = true;
    static const bool logSprites = false;
    static const string title;
    
    static const vector<string> shaderFiles;
    
    static inline Vec2 getScreenCenter()
    {
        return Vec2(width/2, height/2);
    }
    
	static float getScale();

    App();
    virtual ~App();

    virtual void initGLContextAttrs();
    
    void loadShaders();
    
    //globals exposed by app
    ControlRegister* control_register = nullptr;
    Dialog* dialog = nullptr;
    HUD* hud = nullptr;
    Lua::Inst lua;
    GSpace* space = nullptr;
	PlayerCharacter crntPC = PlayerCharacter::flandre;
    PlayScene* playScene = nullptr;

    boost::random::uniform_01<float> randomFloat;
    boost::random::uniform_int_distribution<int> randomInt;
    boost::random::mt19937 randomEngine;

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
    inline void popScene()
    {
        Director::getInstance()->popScene();
    }

    template <typename T, typename... Args>
    inline void runScene(Args... args)
    {
        Director::getInstance()->runScene(createSceneFromLayer<T>(args...));
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
    
    void checkPendingScript();
protected:
    void update(float dt);

    //The shell that is installed in the current scene.
    LuaShell* luaShell;
    string pendingScript;
    
    Scene* createSceneFromLayer(Layer* layer);

    template <typename T, typename... Args>
    inline Scene* createSceneFromLayer(Args... args)
    {
        return createSceneFromLayer(Node::ccCreate<T>(args...));
    }
    
    void installLuaShell(GScene* scene);
};

#endif // _APP_DELEGATE_H_

