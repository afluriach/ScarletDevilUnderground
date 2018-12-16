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

	static bool fullscreen;
	
	static unsigned int framesPerSecond;
	static double secondsPerFrame;
	static boost::rational<int> secondsPerFrameRational;

	static const unsigned int baseWidth = 1600;
     
    constexpr static SpaceFloat Gaccel = 9.806;
    
    static const int pixelsPerTile = 128;
    constexpr static float tilesPerPixel = 1.0f/pixelsPerTile;
    constexpr static float viewWidth = 22.5f;
    
    static const bool showStats = true;
    static const bool logSprites = false;
    static const string title;
    
    static const vector<string> shaderFiles;
    
	static Vec2 getScreenCenter();    
	static float getScale();

	static void setFullscreen(bool fs);
	static void setResolution(unsigned int width, unsigned int height);
	static void setFramerate(unsigned int fps);

	//Methods for controlling the active scene; wraps calls to Director.
	static void runTitleScene();
	static void restartScene();
	static GScene* getCrntScene();
	static void resumeScene();
	static inline void popScene();

	template <typename T, typename... Args>
	static inline void runScene(Args... args)
	{
		Director::getInstance()->runScene(
			Node::ccCreate<T>(args...)
		);
	}

	static void end();

    App();
    virtual ~App();

    virtual void initGLContextAttrs();
    
    void loadShaders();

	void setPlayer(int id);
    
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

    virtual bool applicationDidFinishLaunching();
    virtual void applicationDidEnterBackground();
    virtual void applicationWillEnterForeground();
            
    //Generate [min,max)
	float getRandomFloat(float min, float max);
    //Generate [min,max]
	int getRandomInt(int min, int max);
    
protected:
    void update(float dt);
};

#endif // _APP_DELEGATE_H_

