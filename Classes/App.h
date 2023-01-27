#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by Director.
*/

class FileUtilsZip;
class KeyRegister;
class TimerSystem;

#define USE_TIMERS 1

class App : private Application
{
public:
    enum class EventPriorities
    {
        KeyGlobalListenerEvent = 1,
        KeyRegisterEvent,
    };
    
    static const string title;
    
    static const vector<string> shaderFiles;
	static const unordered_map<string, InterfaceFunction> interfaceFuntions;
    
	static const app_params& getParams();
	static Vec2 getScreenCenter();    
	static float getScale();

	static void setSouthpaw(bool b);
	static void clearAllKeys();
	static void clearKeyAction(string keyName);
	static void addKeyAction(string keyName, string actionName);
	static void assignKey(const vector<string>& v);
#if use_gamepad
	static void clearAllButtons();
	static void clearButtonAction(string buttonName);
	static void addButtonAction(string buttonName, string actionName);
	static void assignButton(const vector<string>& v);
#endif

	static void setFullscreen(bool fs);
	static void setVsync(bool v);
	static void setShowTimers(bool v);
	static void setResolution(unsigned int width, unsigned int height);
	static void setFramerate(unsigned int fps);

	static void setPlayer(string id);
	static void setUnlockAllEquips(bool v);
	static void setDifficulty(float scale);

	//Methods for controlling the active scene; wraps calls to Director.
	static void runTitleScene();
    //runs/loads new scene immediately
	static GScene* runPlayScene(string mapName, string start);
    //queues creation of new scene
    static void loadScene(string mapName, string start);
	static void runOpeningScene();
	static void restartScene();
	static GScene* getCrntScene();
	static void resumeScene();
	static void popScene();
	static void runScene(GScene* scene);
	static void popMenu();

	static FileUtilsZip* getFileUtils();

	template <typename T, typename R, typename... Args>
	static inline R* createAndRunSceneAs(Args... args)
	{
		R* result = Node::ccCreate<T>(args...);
		Director::getInstance()->runScene(result);
		return result;
	}

	template <typename T, typename... Args>
	static inline GScene* createAndRunScene(Args... args)
	{
		return createAndRunSceneAs<T, GScene>(args...);
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

    App();
    virtual ~App();

    virtual void initGLContextAttrs();
    
    void loadShaders();
	void loadObjects();
    
    //globals exposed by app
    static unique_ptr<ControlRegister> control_register;
	static unique_ptr<audio_context> audioContext;
	static unique_ptr<GState> crntState;
	static string crntProfileName;
    static unique_ptr<Lua::Inst> lua;
#if USE_TIMERS
	static unique_ptr<TimerSystem> timerSystem;
	static SpaceFloat timerPrintAccumulator;
	static mutex timerMutex;
#endif
    
	static GState* getCrntState();
	static string getProfileName(int profileID);

protected:
	static App* appInst;
	static bool logTimers;

	FileUtilsZip* fileUtils = nullptr;

	virtual bool applicationDidFinishLaunching();
	virtual void applicationDidEnterBackground();
	virtual void applicationWillEnterForeground();

    void update(float dt);
	void loadConfigFile();
#if USE_TIMERS
	void updateTimerSystem();
#endif
};

#endif // _APP_DELEGATE_H_

