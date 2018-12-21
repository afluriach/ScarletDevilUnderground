#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "functional.hpp"
#include "macros.h"
#include "menu_scenes.h"
#include "GState.hpp"
#include "scenes.h"

const string App::title = "Kouma";

const vector<string> App::shaderFiles = {
	"sprite",
	"inverted",
	"hue_shift_left",
	"hue_shift_right" 
};

unsigned int App::width = 1600;
unsigned int App::height = 1000;

bool App::fullscreen = false;

unsigned int App::framesPerSecond = 60;
double App::secondsPerFrame = 1.0 / App::framesPerSecond;
boost::rational<int> App::secondsPerFrameRational(1,App::framesPerSecond);

unique_ptr<ControlRegister> App::control_register;
unique_ptr<Lua::Inst> App::lua;
PlayerCharacter App::crntPC = PlayerCharacter::flandre;

App* App::appInst;

void App::setFullscreen(bool fs)
{
	fullscreen = fs;
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

App::App()
{
    appInst = this;
    //Initialize Lua
	lua = make_unique<Lua::Inst>("app");
	lua->runFile("scripts/init.lua");    
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
    
    //Load profile data
    GState::load();
    
    Director::getInstance()->getScheduler()->schedule(
        bindMethod(&App::update, this),
        this,
        0.0f,
        false,
        "app_update"
    );
    
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

void App::setPlayer(int id)
{
	crntPC = static_cast<PlayerCharacter>(id);
}

//Generate [min,max)
float App::getRandomFloat(float min, float max) {
	float u01 = appInst->randomFloat(appInst->randomEngine);

	return (min + u01 * (max - min));
}

//Generate [min,max]
int App::getRandomInt(int min, int max) {
	return appInst->randomInt(appInst->randomEngine, boost::random::uniform_int_distribution<int>::param_type(min, max));
}


void App::update(float dt)
{
    control_register->update();
}
