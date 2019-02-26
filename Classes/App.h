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
class GState;
class HUD;
class KeyRegister;
class PlayScene;
class TimerSystem;

#define USE_TIMERS 1

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
    
	static const int maxAudioChannels = 128;

    static const bool showStats = true;
    static const bool logSprites = false;
    static const string title;
    
    static const vector<string> shaderFiles;
    
	static Vec2 getScreenCenter();    
	static float getScale();

	static void setFullscreen(bool fs);
	static void setResolution(unsigned int width, unsigned int height);
	static void setFramerate(unsigned int fps);
	static void setPlayer(int id);

	//Methods for controlling the active scene; wraps calls to Director.
	static void runTitleScene();
	static GScene* runOverworldScene();
	static GScene* runOverworldScene(string mapName, string start);
	static void runOpeningScene();
	static void restartScene();
	static GScene* getCrntScene();
	static void resumeScene();
	static void popScene();
	static void runScene(GScene* scene);

	template <typename T, typename... Args>
	static inline GScene* createAndRunScene(Args... args)
	{
		GScene* result = Node::ccCreate<T>(args...);
		Director::getInstance()->runScene(result);
		return result;
	}

	static bool loadProfile(const string& name);
	static bool saveCrntProfile();
	static bool saveProfile(const string& name);

#if USE_TIMERS
	static void setLogTimers(bool);
	static void printTimerInfo();
#endif
	static void printGlDebug();
	static void end();

	static void playSound(const string& path, float volume);
	static void playSoundSpatial(const string& path, SpaceVect pos, SpaceVect vel);
	static void pauseSounds();
	static void resumeSounds();
	static void setSoundListenerPos(SpaceVect pos, SpaceVect vel, SpaceFloat angle);

    App();
    virtual ~App();

    virtual void initGLContextAttrs();
    
    void loadShaders();
    
    //globals exposed by app
    static unique_ptr<ControlRegister> control_register;
	static unique_ptr<GState> crntState;
	static string crntProfileName;
    static unique_ptr<Lua::Inst> lua;
#if USE_TIMERS
	static unique_ptr<TimerSystem> timerSystem;
	static boost::rational<int> timerPrintAccumulator;
	static mutex timerMutex;
#endif
	static PlayerCharacter crntPC;
            
    //Generate [min,max)
	static float getRandomFloat(float min, float max);
    //Generate [min,max]
	static int getRandomInt(int min, int max);
    
	static const string& getBaseDataPath();

protected:
	static App* appInst;
	static bool logTimers;

	string baseDataPath;

	boost::random::uniform_01<float> randomFloat;
	boost::random::uniform_int_distribution<int> randomInt;
	boost::random::mt19937 randomEngine;

	FMOD::System* audioSystem = nullptr;
	map<string, FMOD::Sound*> loadedAudio;
	map<string, FMOD::Sound*> loadedSpatialAudio;


	virtual bool applicationDidFinishLaunching();
	virtual void applicationDidEnterBackground();
	virtual void applicationWillEnterForeground();

    void update(float dt);
#if USE_TIMERS
	void updateTimerSystem();
#endif
};

#endif // _APP_DELEGATE_H_

