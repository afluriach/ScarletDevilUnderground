#include <cstdlib>

#include "App.h"
#include "controls.h"
#include "LuaShell.hpp"
#include "PlayScene.hpp"
#include "menu.h"
#include "util.h"

USING_NS_CC;

App* app;

const std::string App::title = "フランの地下";

App::App() {
    app = this;
    //Initialize Lua
    lua.runFile("scripts/luatest.lua");
    
    std::vector<std::shared_ptr<Lua::Data>> args;
    
    args.push_back(std::make_shared<Lua::Number>(3));
    args.push_back(std::make_shared<Lua::Number>(6));
    
    std::vector<std::shared_ptr<Lua::Data>> results = lua.call("add", args);
    
    foreach(shared_ptr<Lua::Data> d, results)
    {
        log("result %s", d->toStr().c_str());
    }
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
        
        glview->setDesignResolutionSize(width, height, ResolutionPolicy::SHOW_ALL);
    }

    director->setDisplayStats(showStats);
    director->setAnimationInterval(secondsPerFrame);

    //Activate key register.
    keyRegister = new KeyRegister();
    
    //Create title menu scene and run it.
    runScene<TitleMenu>();

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

void App::installLuaShell(Scene* scene)
{
    luaShell = LuaShell::create();
    luaShell->setVisible(false);
    keyListener.addPressListener(
        Keys::backtick,
        [=]() -> void {luaShell->toggleVisible();}
    );
    keyListener.addPressListener(
        Keys::enter,
        [=]() -> void {if(luaShell->isVisible()) luaShell->runText();}
    );
    
    scene->addChild(luaShell, 2);
}