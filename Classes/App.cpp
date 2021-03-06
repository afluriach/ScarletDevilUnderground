#include "Prefix.h"

#include "Agent.hpp"
#include "AI.hpp"
#include "app_constants.hpp"
#include "audio_context.hpp"
#include "FileIO.hpp"
#include "functional.hpp"
#include "GScene.hpp"
#include "LuaAPI.hpp"
#include "menu_scenes.h"
#include "OpeningScene.hpp"
#include "OverworldScene.hpp"
#include "Resources.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

const string App::title = "Kouma";

const vector<string> App::shaderFiles = {
	"hue_shift_left",
	"hue_shift_right", 
	"inverted",
	"radial_gradient",
	"radial_meter",
	"sprite",
};

#define entry_void(s,x) { #s , makeInterfaceFunction(function<void()>(&App::x))}
#define entry_uint(s,x) { #s , makeInterfaceFunction(function<void(unsigned int)>(&App::x))}
#define entry_uint2(s,x) { #s , makeInterfaceFunction(function<void(unsigned int, unsigned int)>(&App::x))}
#define entry_bool(s,x) { #s , makeInterfaceFunction(function<void(bool)>(&App::x))}
#define entry_float(s,x) { #s , makeInterfaceFunction(function<void(float)>(&App::x))}
#define entry_string(s,x) { #s , makeInterfaceFunction(function<void(string)>(&App::x))}
#define entry_string2(s,x) { #s , makeInterfaceFunction(function<void(string,string)>(&App::x))}

const unordered_map<string, InterfaceFunction> App::interfaceFuntions = {
	entry_uint(framerate, setFramerate),
	entry_bool(fullscreen, setFullscreen),
	entry_uint2(resolution, setResolution),
	entry_bool(vsync, setVsync),
	entry_float(difficulty, setDifficulty),
	entry_bool(timers, setShowTimers),

	//Control register config commands
	{ "button", &App::assignButton },
	{ "key", &App::assignKey },
	entry_void(clear_all_keys, clearAllKeys),
	entry_void(clear_all_buttons, clearAllButtons),
	entry_string(clear_button, clearButtonAction),
	entry_string(clear_key, clearKeyAction),
	entry_string2(add_key_action, addKeyAction),
	entry_string2(add_button_action, addButtonAction),
	entry_bool(southpaw, setSouthpaw),
};

unique_ptr<ControlRegister> App::control_register;
unique_ptr<audio_context> App::audioContext;
unique_ptr<GState> App::crntState;
string App::crntProfileName;
unique_ptr<Lua::Inst> App::lua;

#if USE_TIMERS
unique_ptr<TimerSystem> App::timerSystem;
SpaceFloat App::timerPrintAccumulator = 1.0;
mutex App::timerMutex;
#endif

App* App::appInst;
bool App::logTimers = false;

void App::setSouthpaw(bool b)
{
	control_register->setSouthpaw(b);
}

void App::clearAllKeys()
{
	control_register->clearAllKeys();
}

void App::clearKeyAction(string keyName)
{
	control_register->clearKeyAction(keyName);
}

void App::addKeyAction(string keyName, string actionName)
{
	control_register->addKeyAction(keyName, actionName);
}

void App::assignKey(const vector<string>& v)
{
	control_register->assignKey(v);
}

#if use_gamepad
void App::clearAllButtons()
{
	control_register->clearAllButtons();
}

void App::clearButtonAction(string buttonName)
{
	control_register->clearButtonAction(buttonName);
}

void App::addButtonAction(string buttonName, string actionName)
{
	control_register->addButtonAction(buttonName, actionName);
}

void App::assignButton(const vector<string>& v)
{
	control_register->assignButton(v);
}

#endif

void App::setFullscreen(bool fs)
{
	app::params.fullscreen = fs;
}

void App::setVsync(bool v)
{
	app::params.vsync = v;
}

void App::setShowTimers(bool v)
{
	app::params.showTimers = v;
}

void App::setResolution(unsigned int width, unsigned int height)
{
	app::params.width = width;
	app::params.height = height;
}

void App::setFramerate(unsigned int fps)
{
	app::params.framesPerSecond = fps;
	app::params.secondsPerFrame = 1.0 / fps;
}

Vec2 App::getScreenCenter()
{
	return Vec2(app::params.width / 2, app::params.height / 2);
}

const app_params& App::getParams()
{
	return app::params;
}

float App::getScale()
{
	return app::params.getScale();
}

App::App()
{
    appInst = this;

    io::checkCreateSubfolders();

	LogSystem::initThread();

	log("\nKouma started at %s", getNowTimestamp());

#if !DEV_MODE
	//Initialize resources.zip filesystem
	fileUtils = new FileUtilsZip();
	fileUtils->init();
	FileUtils::setDelegate(fileUtils);
#endif

	//Activate key register.
	control_register = make_unique<ControlRegister>();
	audioContext = make_unique<audio_context>();

    //Initialize Lua
#if DEV_MODE
	lua = make_unique<Lua::Inst>("app");
	lua->runFile("scripts/init.lua");    
#endif

#if USE_TIMERS
	timerSystem = make_unique<TimerSystem>();
#endif

	GState::initProfiles();
	loadConfigFile();
	loadObjects();
}

App::~App() 
{
	control_register = nullptr;
	
	//Close AL
	audioContext.reset();

	if (fileUtils) {
		FileUtils::setDelegate(nullptr);
		//un-setting the delegate deletes it
		//delete fileUtils;
		fileUtils = nullptr;
	}

    log("app exiting");
	LogSystem::exit();
}

//Called in AppController.mm. This appears to be for Mac/iOS only.
void App::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool App::applicationDidFinishLaunching() {
#ifdef _WIN32
	GLViewImpl::vsync = vsync;
	Application::vsync = vsync;
#endif

    //Initialize OpenGL view (and set window title on desktop version).
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = app::params.fullscreen ?
            GLViewImpl::createWithFullScreen(App::title) :
            GLViewImpl::createWithRect(App::title, cocos2d::CCRect(0,0, app::params.width, app::params.height))
        ;
        
        director->setOpenGLView(glview);
        //director->setContentScaleFactor(1.0f);
        
        glview->setDesignResolutionSize(app::params.width, app::params.height, ResolutionPolicy::SHOW_ALL);
    }

	if(app::params.showTimers)
		director->setDisplayStats(true);

    director->setAnimationInterval(app::params.secondsPerFrame);

    loadShaders();
   
	crntState = make_unique<GState>();
	io::getProfiles();

    Director::getInstance()->getScheduler()->schedule(
        bindMethod(&App::update, this),
        this,
        0.0f,
        false,
        "app_update"
    );

	audioContext->initAudio();

    //Create title menu scene and run it.
    runTitleScene();

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void App::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void App::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

void App::printGlDebug()
{
	//print the program, vertex, and fragment log for each custom shader.

	log("printGlDebug:");

	for (const string& name : App::shaderFiles) {
		log("%s", name.c_str());
		GLProgram* p = GLProgramCache::getInstance()->getGLProgram(name);

		string logtext;

		logtext = p->getProgramLog();
		if (!logtext.empty())
			log("%s program log: \n%s", name.c_str(), logtext.c_str());

		logtext = p->getVertexShaderLog();
		if (!logtext.empty())
			log("%s vertex log: \n%s", name.c_str(), logtext.c_str());

		logtext = p->getFragmentShaderLog();
		if (!logtext.empty())
			log("%s fragment log: \n%s", name.c_str(), logtext.c_str());

	}
}

void App::end()
{
    Director::getInstance()->end();
}

void App::loadShaders()
{
    for(const string& name: shaderFiles){
        GLProgramCache::getInstance()->loadGLProgram(name, "shaders/"+name+".vert", "shaders/"+name+".frag");
    }
}

void App::loadObjects()
{
	GSpace::loadScriptVM();

	GObject::initObjectInfo();
	GObject::initNameMap();

	app::loadSprites();
	app::loadLights();

	app::loadAreas();
	app::loadBombs();
	app::loadBullets();
	app::loadEffects();
	app::loadSpells();
	//Collectibles need to be loaded after MagicEffects.
	app::loadFirePatterns();
	app::loadEffectAreas();
	app::loadEnvironmentObjects();
	app::loadFloors();
	app::loadItems();
	app::loadEnemies();
	app::loadNPCs();
	app::loadPlayers();

	GSpace::playerCharacter = app::getPlayer("FlandrePC");
}

void App::runTitleScene()
{
	createAndRunScene<TitleMenuScene>();
}

void App::runOpeningScene()
{
	createAndRunScene<OpeningScene>();
}

GScene* App::runOverworldScene()
{
	return runOverworldScene("mansion_b1", "player_start");
}

GScene* App::runOverworldScene(string mapName, string start)
{
	return createAndRunScene<OverworldScene>("overworld/"+mapName, start);
}

GScene* App::getCrntScene()
{
	return dynamic_cast<GScene*>(Director::getInstance()->getRunningScene());
}

void App::restartScene()
{
	GScene* crntScene = getCrntScene();

	if (crntScene)
		Director::getInstance()->runScene(crntScene->getReplacementScene());
}

void App::popScene()
{
	Director::getInstance()->popScene();
}

void App::resumeScene()
{
	getCrntScene()->exitPause();
}

void App::runScene(GScene* scene)
{
	Director::getInstance()->runScene(scene);
}

void App::popMenu()
{
	App::getCrntScene()->popMenu();
}

bool App::loadProfile(const string& name)
{
	unique_ptr<GState> result = io::loadProfileState(name);

	if (result.get()){
		crntProfileName = name;
		crntState = move(result);
		return true;
	}
	else {
		return false;
	}
}

bool App::saveCrntProfile()
{
	return saveProfile(crntProfileName);
}

bool App::saveProfile(const string& name)
{
	if (name.empty()) {
		log("Can't save, empty profile name.");
		return false;
	}

	return io::saveProfileState(crntState.get(), name);
}

bool App::autosaveProfile()
{
	return io::saveProfileState(crntState.get(), "autosave");
}

void App::setPlayer(string id)
{
	GSpace::playerCharacter = app::getPlayer(id);
}

void App::setUnlockAllEquips(bool v)
{
	app::params.unlockAllEquips = v;
}

void App::setDifficulty(float scale)
{
	if (scale >= 0.25f && scale <= 4.0f) {
		app::params.difficultyScale = scale;
	}
	else {
		log("Invalid difficulty scale %f!", scale);
	}
}

GState* App::getCrntState() {
	return crntState.get();
}

FileUtilsZip* App::getFileUtils()
{
	return appInst->fileUtils;
}

void App::update(float dt)
{
    control_register->update();
	audioContext->update();

#if USE_TIMERS
	updateTimerSystem();
#endif
}

void App::loadConfigFile()
{
	string filepath = io::getConfigFilePath();
	string file;

	if (FileUtils::getInstance()->isFileExist(filepath)) {
		file = io::loadTextFile(filepath);
		log("Loading \"%s\"", filepath.c_str());
	}
	else {
		log("config.txt not found");
		return;
	}

	vector<string> lines = splitString(file, "\n\r");

	for (string line : lines)
	{
		vector<string> tokens = splitString(line, " \t");
		if (tokens.empty() || isComment(tokens.at(0)))
			continue;

		auto it = interfaceFuntions.find(tokens.at(0));
		if (it != interfaceFuntions.end()) {
			try {
				it->second(tokens);
			}
			catch (boost::bad_lexical_cast e) {
				log("config.txt: invalid line \"%s\".", line.c_str());
			}
		}
	}
}

#if USE_TIMERS
void App::updateTimerSystem()
{
	timerMutex.lock();
	timerSystem->addEntry(TimerType::draw, Director::getInstance()->getRenderTimes().back());

	if (logTimers)
	{
		timerDecrement(timerPrintAccumulator);

		if (timerPrintAccumulator <= 0) {
			printTimerInfo();
			timerPrintAccumulator = 1.0;
		}
	}
	timerMutex.unlock();
}

void App::setLogTimers(bool b)
{
	logTimers = b;
}

void App::printTimerInfo()
{
	if (!timerSystem) {
		log("App::printTimerInfo: timer system not available!");
		return;
	}

	boost::circular_buffer<std::chrono::duration<long, std::micro>> _render = Director::getInstance()->getRenderTimes();

	TimerSystem::printTimerStats(timerSystem->getStats(TimerType::gobject), "Object Update");
	TimerSystem::printTimerStats(timerSystem->getStats(TimerType::physics), "Physics");
	TimerSystem::printTimerStats(TimerSystem::getBufferStats(_render), "Render");
}
#endif
