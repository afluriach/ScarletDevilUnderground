#include "Prefix.h"

//Already included in PCH
//#include "App.h"

App* app;

const string App::title = "フランの地下";

App::App() : lua("app"), replInst("repl")
{
    app = this;
    //Initialize Lua
    lua.runFile("scripts/init.lua");
    
    luaReplThread = new thread(Lua::replThreadMain, &replInst);
}

App::~App() 
{
    delete keyRegister;
    
    try{
        lstop(replInst.state, nullptr);
    }
    catch(lua_longjmp* ex){}
    luaReplThread->join();
    delete luaReplThread;
    
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
        glview = GLViewImpl::createWithRect(App::title, cocos2d::Rect(0,0,App::width, App::height),screenscale);
        director->setOpenGLView(glview);
        
        glview->setDesignResolutionSize(width, height, ResolutionPolicy::SHOW_ALL);
    }

    director->setDisplayStats(showStats);
    director->setAnimationInterval(secondsPerFrame);

    //Activate key register.
    keyRegister = new KeyRegister();
    
    //Create title menu scene and run it.
    runScene<TitleMenu>();
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

Scene* App::createSceneFromLayer(Layer* layer)
{
    Scene* scene  = Scene::create();
    scene->addChild(layer,1);
    
    GScene* gscene = dynamic_cast<GScene*>(layer);
    if(gscene)
        installLuaShell(gscene);
    else
        log("createSceneFromLayer: Lua shell not installed for non-GScene.");
    
    return scene;
}


void App::end()
{
    Director::getInstance()->end();
}

void App::checkPendingScript()
{
    if(!pendingScript.empty()){
        lua.runString(pendingScript);
        pendingScript.clear();
    }
    Lua::Inst::runCommands();
}

void App::installLuaShell(GScene* gscene)
{
    luaShell = LuaShell::create();
    luaShell->setVisible(false);
    keyListener.addPressListener(
        Keys::backtick,
        [=]() -> void {luaShell->toggleVisible();}
    );
    keyListener.addPressListener(
        Keys::enter,
        [=]() -> void {if(luaShell->isVisible()) pendingScript = luaShell->getText();}
    );
    
    gscene->multiUpdate.insertWithOrder(bind(&App::checkPendingScript,this), GScene::updateOrder::runShellScript);
    
    gscene->getLayer(GScene::sceneLayers::luaShellLayer)->addChild(luaShell, 1);
}