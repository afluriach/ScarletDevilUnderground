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
#include "Resources.hpp"
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
	"sfx/bomb_explosion1.wav",
	"sfx/bomb_explosion2.wav",
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
	"sfx/red_fairy_explosion.wav",
	"sfx/shot.wav",
};

template<typename T>
tuple<T> parse(const vector<string>& _v, int idx)
{
	return tuple<T>(boost::lexical_cast<T>(_v.at(idx)));
}

template <typename Crnt, typename Next, typename... Rest>
tuple<Crnt, Next, Rest...> parse(const vector<string>& _v, int idx)
{
	return tuple_cat(
		parse<Crnt>(_v, idx),
		parse<Next, Rest...>(_v, idx + 1)
	);
}

template<typename... T>
void callAdapter(function<void(T...)> static_method, vector<string> tokens)
{
	size_t n_args = tuple_size<tuple<T...>>::value;

	if (tokens.empty()) {
		log("callAdapter: empty input");
		return;
	}
	else if (tokens.size() - 1 != n_args) {
		log("callAdapter: %d arguments expected, %d found", n_args, tokens.size() - 1);
		return;
	}

	tuple<T...> args = parse<T...>(tokens, 1);
	variadic_call<void>(static_method, args);
}

template<typename... T>
InterfaceFunction makeInterfaceFunction( function<void(T...)> static_method )
{
	return [static_method](const vector<string>& tokens)->void {
		callAdapter(static_method, tokens);
	};
}

#define entry(x) { #x , makeInterfaceFunction(function(&App::x))}

const unordered_map<string, InterfaceFunction> App::interfaceFuntions = {
	entry(setFramerate),
	entry(setFullscreen),
	entry(setMultithread),
	entry(setResolution),
	entry(setVsync),
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
mutex App::audioMutex;

App* App::appInst;
bool App::logTimers = false;

void App::clearAllKeys()
{
	control_register->clearAllKeys();
}

void App::clearKeyAction(const string& keyName)
{
	control_register->clearKeyAction(keyName);
}

void App::addKeyAction(const string& keyName, const string& actionName)
{
	control_register->addKeyAction(keyName, actionName);
}

#if use_gamepad
void App::clearAllButtons()
{
	control_register->clearAllButtons();
}

void App::clearButtonAction(const string& buttonName)
{
	control_register->clearButtonAction(buttonName);
}

void App::addButtonAction(const string& buttonName, const string& actionName)
{
	control_register->addButtonAction(buttonName, actionName);
}
#endif

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
	appInst->audioDevice = alcOpenDevice(nullptr);
	if (!appInst->audioDevice) {
		log("Failed to open audio device");
		return;
	}

	appInst->audioContext = alcCreateContext(appInst->audioDevice, nullptr);
	if (!appInst->audioContext) {
		log("Failed to open audio context.");
		return;
	}

	if (!alcMakeContextCurrent(appInst->audioContext)) {
		log("Failed to set current context.");
		return;
	}

	alDistanceModel(AL_EXPONENT_DISTANCE);

	for (string s : soundFiles) {
		loadSound(s);
	}
}

ALuint App::initSoundSource(const Vec3& pos, const Vec3& vel, bool relative)
{
	ALuint result = 0;

	alGenSources(1, &result);
	alSource3f(result, AL_POSITION, pos.x, pos.y, pos.z);
	alSource3f(result, AL_VELOCITY, vel.x, vel.y, vel.z);
	if(relative)
		alSourcei(result, AL_SOURCE_RELATIVE, AL_TRUE);

	return result;
}

void App::loadSound(const string& path)
{
	SF_INFO info;
	SNDFILE* file = nullptr;
	if (appInst->fileUtils) {
		file = appInst->fileUtils->openSoundFile(path, &info);
	}

	if(!file){
		file = sf_open(path.c_str(), SFM_READ, &info);
	}
	ALuint bufferID;

	if (!file) {
		log("Failed to load audio file %s.", path.c_str());
		return;
	}

	if (info.channels != 1) {
		log("Sound file %s is multi-channel!", path.c_str());
		sf_close(file);
		return;
	}

	short* buf = new short[info.frames];
	sf_count_t bufPos = 0;

	while (true) {
		if (bufPos >= info.frames) break;

		sf_count_t readSize = sf_read_short(file, buf + bufPos, info.frames - bufPos);

		if (readSize > 0) bufPos += readSize;
		else break;
	}

	alGenBuffers(1, &bufferID);

	if (bufferID == AL_INVALID_VALUE) {
		log("Failed to create sound buffer.");
		return;
	}

	alBufferData(
		bufferID,
		AL_FORMAT_MONO16,
		buf,
		info.frames,
		info.samplerate
	);

	appInst->loadedBuffers.insert_or_assign(path, bufferID);
	delete[] buf;

	if (appInst->fileUtils) {
		appInst->fileUtils->closeSoundFile(path);
		appInst->fileUtils->unloadFile(path);
	}
}

ALuint App::playSound(const string& path, float volume)
{
	audioMutex.lock();

	ALuint source = initSoundSource(Vec3::ZERO, Vec3::ZERO, true);
	ALuint sound = getOrDefault(appInst->loadedBuffers, path, to_uint(0));

	if (source != 0 && sound != 0) {
		alSourcei(source, AL_BUFFER, sound);
		alSourcePlay(source);
		appInst->activeSources.insert(source);
	}

	audioMutex.unlock();

	return source;
}

ALuint App::playSoundSpatial(const string& path, const Vec3& pos, const Vec3& vel, float volume, bool loop)
{
	audioMutex.lock();

	ALuint bufferID = getOrDefault(appInst->loadedBuffers, path, to_uint(0));
	ALuint source = 0;

	if (bufferID == 0) {
		log("Unknown audio file %s", path.c_str());
	}
	else {
		source = initSoundSource(pos, vel, false);
	}

	if (source != 0) {
		alSourcei(source, AL_BUFFER, bufferID);
		alSourcef(source, AL_ROLLOFF_FACTOR, boost::math::float_constants::root_two);
		alSourcei(source, AL_LOOPING, loop);
		alSourcePlay(source);
		appInst->activeSources.insert(source);
	}

	audioMutex.unlock();

	return source;
}

void App::endSound(ALuint source)
{
	audioMutex.lock();
	auto it = appInst->activeSources.find(source);
	if (it != appInst->activeSources.end()) {
		alSourceStop(*it);
		appInst->activeSources.erase(it);
	}
	audioMutex.unlock();
}

void App::pauseSounds()
{
	audioMutex.lock();
	for (ALuint sourceID : appInst->activeSources)
	{
		ALenum state;
		alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
		if (state != AL_STOPPED && state != AL_PAUSED) {
			alSourcePause(sourceID);
		}
	}
	audioMutex.unlock();
}

void App::resumeSounds()
{
	audioMutex.lock();
	for (ALuint sourceID : appInst->activeSources)
	{
		ALenum state;
		alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED) {
			alSourcePlay(sourceID);
		}
	}
	audioMutex.unlock();
}

void App::setSoundListenerPos(SpaceVect pos, SpaceVect vel, SpaceFloat angle)
{
	audioMutex.lock();
	Vec3 _pos = toVec3(pos);
	Vec3 _vel = toVec3(vel);
	array<float, 6> orientation = {
		0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f
	};

	alListenerfv(AL_POSITION, &_pos.x);
	alListenerfv(AL_VELOCITY, &_vel.x);
	alListenerfv(AL_ORIENTATION, &orientation[0]);
	audioMutex.unlock();
}

bool App::setSoundSourcePos(ALuint source, SpaceVect pos, SpaceVect vel, SpaceFloat angle)
{
	bool valid = false;
	Vec3 _pos = toVec3(pos);
	Vec3 _vel = toVec3(vel);
	audioMutex.lock();

	if (appInst->activeSources.find(source) != appInst->activeSources.end()) {
		alSourcefv(source, AL_POSITION, &_pos.x);
		alSourcefv(source, AL_VELOCITY, &_vel.x);
		valid = true;
	}

	audioMutex.unlock();
	return valid;
}

bool App::isSoundSourceActive(ALuint source)
{
	bool result;
	audioMutex.lock();
	result = appInst->activeSources.find(source) != appInst->activeSources.end();
	audioMutex.unlock();
	return result;
}

App::App()
{
    appInst = this;

	//Initialize resources.zip filesystem
	fileUtils = new FileUtilsZip();
	fileUtils->init();
	FileUtils::setDelegate(fileUtils);

	//Activate key register.
	control_register = make_unique<ControlRegister>();

    //Initialize Lua
	lua = make_unique<Lua::Inst>("app");
	lua->runFile("scripts/init.lua");    

#if USE_TIMERS
	timerSystem = make_unique<TimerSystem>();
#endif

	baseDataPath = FileUtils::getInstance()->getWritablePath();
	GState::initProfiles();
	loadConfigFile();

	if (FileUtils::getInstance()->isFileExist(io::getControlMappingPath())) {
		control_register->applyControlSettings(io::loadTextFile(io::getControlMappingPath()));
	}
}

App::~App() 
{
	control_register = nullptr;
	
	//Close AL
	alcMakeContextCurrent(nullptr);
	if (audioContext)
		alcDestroyContext(audioContext);
	if (audioDevice)
		alcCloseDevice(audioDevice);

	if (fileUtils) {
		FileUtils::setDelegate(nullptr);
		//un-setting the delegate deletes it
		//delete fileUtils;
		fileUtils = nullptr;
	}

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

	audioMutex.lock();

	auto it = activeSources.begin();
	while (it != activeSources.end()) {
		ALenum state;
		alGetSourcei(*it, AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED) {
			alDeleteSources(1, &(*it));
			it = activeSources.erase(it);
		}
		else {
			++it;
		}
	}
	audioMutex.unlock();

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
			timerPrintAccumulator = boost::rational<int>(1);
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
