#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "functional.hpp"
#include "macros.h"
#include "menu.h"
#include "GState.hpp"
#include "scenes.h"

App* app;

const string App::title = "Kouma";

const vector<string> App::shaderFiles = boost::assign::list_of
("sprite")
("inverted")
("hue_shift_left")
("hue_shift_right")
;

unsigned int App::width = 1600;
unsigned int App::height = 1000;

bool App::fullscreen = false;

unsigned int App::framesPerSecond = 60;
double App::secondsPerFrame = 1.0 / App::framesPerSecond;
boost::rational<int> App::secondsPerFrameRational(1,App::framesPerSecond);

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

float App::getScale()
{
	return 1.0f * width / baseWidth;
}

App::App() : lua("app")
{
    app = this;
    //Initialize Lua
    lua.runFile("scripts/init.lua");    
}

App::~App() 
{
    delete_if(control_register);
        
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
    control_register = new ControlRegister();
    
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
    lua.runFile("scripts/title.lua");

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

void App::end()
{
    Director::getInstance()->end();
}

void App::loadShaders()
{
    BOOST_FOREACH(string name, shaderFiles){
        GLProgramCache::getInstance()->loadGLProgram(name, "shaders/"+name+".vert", "shaders/"+name+".frag");
    }
}

void App::runTitleScene()
{
	runScene<TitleMenuScene>();
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

void App::setPlayer(int id)
{
	crntPC = static_cast<PlayerCharacter>(id);
}

void App::update(float dt)
{
    control_register->update();
}
