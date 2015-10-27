#include <cstdlib>

#include "AppDelegate.h"
#include "controls.h"
#include "GameplayScene.hpp"
#include "menu.h"
#include "util.h"

USING_NS_CC;

AppDelegate* app;

const std::string AppDelegate::title = "フランの地下";

AppDelegate::AppDelegate() {
    app = this;
}

AppDelegate::~AppDelegate() 
{
    delete keyRegister;
    
    log("app exiting");
}

//Called in AppController.mm. This appears to be for Mac/iOS only.
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
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
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
