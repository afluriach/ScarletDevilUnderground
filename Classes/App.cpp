#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "FileIO.hpp"
#include "functional.hpp"
#include "GScene.hpp"
#include "GState.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "menu_scenes.h"
#include "OpeningScene.hpp"
#include "OverworldScene.hpp"
#include "util.h"

const string App::title = "Kouma";

const vector<string> App::shaderFiles = {
	"agent_overlay",
	"cone",
	"hue_shift_left",
	"hue_shift_right", 
	"inverted",
	"radial_gradient",
	"radial_meter",
	"sprite",
};

const vector<string> App::soundFiles = {
	"sfx/bomb_explosion.wav",
	"sfx/bomb_fuse.wav",
	"sfx/enemy_damage.wav",
	"sfx/footstep_cave.wav",
	"sfx/footstep_grass.wav",
	"sfx/footstep_ice.wav",
	"sfx/footstep_sand.wav",
	"sfx/footstep_stone.wav",
	"sfx/graze.wav",
	"sfx/player_damage.wav",
	"sfx/player_death.wav",
	"sfx/player_power_attack.wav",
	"sfx/player_spellcard.wav",
	"sfx/powerup.wav",
	"sfx/shot.wav",
};

unsigned int App::width = 1600;
unsigned int App::height = 1000;

bool App::fullscreen = false;
bool App::vsync = true;
bool App::multithread = true;

bool App::unlockAllEquips = false;

unsigned int App::framesPerSecond = 60;
double App::secondsPerFrame = 1.0 / App::framesPerSecond;
boost::rational<int> App::secondsPerFrameRational(1,App::framesPerSecond);

unique_ptr<ControlRegister> App::control_register;
unique_ptr<GState> App::crntState;
string App::crntProfileName;
unique_ptr<Lua::Inst> App::lua;
PlayerCharacter App::crntPC = PlayerCharacter::flandre;

#if USE_TIMERS
unique_ptr<TimerSystem> App::timerSystem;
boost::rational<int> App::timerPrintAccumulator(1);
mutex App::timerMutex;
#endif

App* App::appInst;
bool App::logTimers = false;

void App::setFullscreen(bool fs)
{
	fullscreen = fs;
}

void App::setVsync(bool v)
{
	vsync = v;
}

void App::setMultithread(bool v)
{
	multithread = v;
}

void App::setResolution(unsigned int width, unsigned int height)
{
	App::width = width;
	App::height = height;
}

void App::setFramerate(unsigned int fps)
{
	framesPerSecond = fps;
	secondsPerFrame = 1.0 / fps;
	secondsPerFrameRational = boost::rational<int>(1, fps);
}

Vec2 App::getScreenCenter()
{
	return Vec2(width / 2, height / 2);
}


float App::getScale()
{
	return 1.0f * width / baseWidth;
}

void App::initAudio()
{
	FMOD_RESULT result = FMOD::System_Create(&appInst->audioSystem);

	if (result != FMOD_OK) {
		log("FMOD system failed to create!");
		return;
	}

	result = appInst->audioSystem->init(maxAudioChannels, FMOD_INIT_NORMAL, 0);

	if (result != FMOD_OK) {
		log("FMOD system failed to initialize!");
		return;
	}

	for (string sound : soundFiles)
	{
		loadSound(sound);
	}
}

void App::loadSound(const string& path)
{
	FMOD::Sound *audio;
	FMOD_RESULT result = appInst->audioSystem->createSound(path.c_str(), FMOD_DEFAULT, 0, &audio);

	if (result == FMOD_OK) {
		appInst->loadedAudio.insert_or_assign(path, audio);
	}
	else {
		log("Failed to create sound %s, error code %d.", path.c_str(), to_int(result));
	}
}

void App::playSound(const string& path, float volume)
{
	auto it = appInst->loadedAudio.find(path);

	if (it == appInst->loadedAudio.end()) {
		log("Unknown sound %s!", path.c_str());
		return;
	}

	FMOD::Channel* ch;
	appInst->audioSystem->playSound(it->second, nullptr, true, &ch);
	
	ch->setMode(FMOD_DEFAULT);
	ch->setVolume(volume);
	ch->setPaused(false);
}

void App::playSoundSpatial(const string& path, FMOD_VECTOR pos, FMOD_VECTOR vel, float volume)
{
	auto it = appInst->loadedAudio.find(path);

	if (it == appInst->loadedAudio.end()) {
		log("Unknown sound %s!", path.c_str());
		return;
	}

	FMOD::Channel* ch;
	appInst->audioSystem->playSound(it->second, nullptr, true, &ch);
	
	ch->setMode(FMOD_3D);
	ch->set3DMinMaxDistance(0.5f, 15.0f);
	ch->set3DAttributes(&pos, &vel);
	ch->setVolume(volume);
	ch->setPaused(false);
}

void App::pauseSounds()
{
	if (appInst->audioSystem) {
		FMOD::ChannelGroup* cg;
		appInst->audioSystem->getMasterChannelGroup(&cg);

		cg->setPaused(true);
	}
}

void App::resumeSounds()
{
	if (appInst->audioSystem) {
		FMOD::ChannelGroup* cg;
		appInst->audioSystem->getMasterChannelGroup(&cg);

		cg->setPaused(false);
	}
}

void App::setSoundListenerPos(SpaceVect pos, SpaceVect vel, SpaceFloat angle)
{
	FMOD_VECTOR _pos = toFmod(pos);
	FMOD_VECTOR _vel = toFmod(vel);
	FMOD_VECTOR facing = {0.0f, 0.0f, 1.0f};
	FMOD_VECTOR up = {0.0f, 1.0f, 0.0f};

	if (appInst->audioSystem) {
		appInst->audioSystem->set3DListenerAttributes(0, &_pos, &_vel, &facing, &up);
	}
}

App::App()
{
    appInst = this;
    //Initialize Lua
	lua = make_unique<Lua::Inst>("app");
	lua->runFile("scripts/init.lua");    

#if USE_TIMERS
	timerSystem = make_unique<TimerSystem>();
#endif

	baseDataPath = FileUtils::getInstance()->getWritablePath();
	GState::initProfiles();
}

App::~App() 
{
	control_register = nullptr;

    log("app exiting");
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
	GLViewImpl::vsync = vsync;
	Application::vsync = vsync;

    //Initialize OpenGL view (and set window title on desktop version).
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = fullscreen ?
            GLViewImpl::createWithFullScreen(App::title) :
            GLViewImpl::createWithRect(App::title, cocos2d::CCRect(0,0,App::width, App::height))
        ;
        
        director->setOpenGLView(glview);
        //director->setContentScaleFactor(1.0f);
        
        glview->setDesignResolutionSize(width, height, ResolutionPolicy::SHOW_ALL);
    }

    director->setDisplayStats(showStats);
    director->setAnimationInterval(secondsPerFrame);

    loadShaders();

    //Activate key register.
    control_register = make_unique<ControlRegister>();
   
	crntState = make_unique<GState>();
	io::getProfiles();

    Director::getInstance()->getScheduler()->schedule(
        bindMethod(&App::update, this),
        this,
        0.0f,
        false,
        "app_update"
    );

	initAudio();

    //Create title menu scene and run it.
    runTitleScene();
    lua->runFile("scripts/title.lua");

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
	if (appInst->audioSystem) {

		for (auto entry : appInst->loadedAudio) {
			entry.second->release();
		}

		appInst->audioSystem->release();
	}

    Director::getInstance()->end();
}

void App::loadShaders()
{
    for(const string& name: shaderFiles){
        GLProgramCache::getInstance()->loadGLProgram(name, "shaders/"+name+".vert", "shaders/"+name+".frag");
    }
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

	io::checkCreateSubfolders();
	return io::saveProfileState(crntState.get(), name);
}

bool App::autosaveProfile()
{
	io::checkCreateSubfolders();
	return io::saveProfileState(crntState.get(), "autosave");
}

void App::setPlayer(int id)
{
	crntPC = static_cast<PlayerCharacter>(id);
}

void App::setUnlockAllEquips(bool v)
{
	unlockAllEquips = v;
}

const string& App::getBaseDataPath() {
	return appInst->baseDataPath;
}

GState* App::getCrntState() {
	return crntState.get();
}

void App::update(float dt)
{
    control_register->update();

	if (audioSystem) {
		audioSystem->update();
	}

#if USE_TIMERS
	updateTimerSystem();
#endif
}

#if USE_TIMERS
void App::updateTimerSystem()
{
	timerSystem->addEntry(TimerType::draw, Director::getInstance()->getRenderTimes().back());

	if (logTimers)
	{
		timerDecrement(timerPrintAccumulator);

		if (timerPrintAccumulator <= 0) {
			timerMutex.lock();
			printTimerInfo();
			timerMutex.unlock();
			timerPrintAccumulator = boost::rational<int>(1);
		}
	}
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
