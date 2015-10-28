#include <cstdlib>

#include "App.h"
#include "controls.h"
#include "PlayScene.hpp"
#include "menu.h"
#include "util.h"

USING_NS_CC;

App* app;

const std::string App::title = "フランの地下";

App::App() {
    app = this;
}

App::~App() 
{
    delete keyRegister;
    
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
        glview = GLViewImpl::create(title);
        director->setOpenGLView(glview);
    }

    director->setDisplayStats(showStats);
    director->setAnimationInterval(secondsPerFrame);

    //Activate key register.
    keyRegister = new KeyRegister();
    
    //Create title menu scene and run it.
    auto scene = createSceneFromLayer<TitleMenu>();
    director->runWithScene(scene);

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
