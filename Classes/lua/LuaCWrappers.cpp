//
//  LuaCWrappers.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "App.h"
#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "LuaWrap.h"
#include "Player.hpp"
#include "PlayScene.hpp"

namespace Lua{

unordered_map<string, Class> Class::classes;
bool Class::init = false;

void createObject(ValueMap args)
{
    if(!app->space) throw lua_runtime_error("createObject: Cannot access objects in this scene.");
    
    app->space->createObject(args);
}

void removeObject(string name)
{
    if(!app->space) throw lua_runtime_error("removeObject: Cannot access objects in this scene.");
    
    app->space->removeObject(name);
}

void setpos(string name, float x, float y)
{
    GObject* obj = app->space->getObject(name);
    
    if(obj)
        obj->body->setPos(SpaceVect(x,y));
    else
        throw runtime_error("setpos: " + name + " not found");
}

void setvel(string name, float x, float y)
{
    GObject* obj = app->space->getObject(name);

    if(obj)
        obj->body->setVel(SpaceVect(x,y));
    else
        throw runtime_error("setvel: " + name + " not found");
}

int getObjectCount()
{
    if(!app->space) throw lua_runtime_error("createObject: Cannot access objects in this scene.");
        
    return app->space->getObjectCount();
}

unordered_map<int, string> getUUIDNameMap()
{
    GSpace* space = app->space;
    
    if(!space) return unordered_map<int,string>();
    
    return space->getUUIDNameMap();
}

void printMap(unordered_map<string,string> m)
{
    for(auto it = m.begin(); it != m.end(); ++it){
        log("%s : %s", it->first.c_str(), it->second.c_str());
    }
}

void addUpdate(function<void()> f, int order)
{
    GScene::crntScene->multiUpdate.insertWithOrder(
        [=]() -> void{
            f();
        },
        order
    );
}

//Increase window size and scale up content to match.
void setscreenscale(float f)
{
    if(f <= 0) throw lua_runtime_error("setscreenscale: must be positive");

    app->screenscale = f;
}

//Scales graphics content resoltuion without changing window resolution;
//for systems where the window resolution does not actually match content resolution
//(e.g. Retina displays).
void setdpiscale(float f)
{
    if(f <= 0) throw lua_runtime_error("setdpiscale: must be positive");

    app->dpiscale = f;
}

void setFullscreen(bool fs)
{
    app->fullscreen = fs;
}

void setResolution(unsigned int width, unsigned int height)
{
	App::width = width;
	App::height = height;
}

void setFramerate(unsigned int fps)
{
	App::framesPerSecond = fps;
	App::secondsPerFrame = 1.0 / fps;
	App::secondsPerFrameRational = boost::rational<int>(1, fps);
}

GObject* getObjByName(string name)
{
    if(!app->space) throw lua_runtime_error("getObjByName: Cannot access objects in this scene.");
    
    return app->space->getObject(name);
}

vector<string> getObjectNames()
{
    if(!app->space) throw lua_runtime_error("getObjectNames: Cannot access objects in this scene.");

    return app->space->getObjectNames();
}

//The Lua conversion template will convert the wrapper object to a valid pointer or null.
bool isValidObject(GObject* object)
{
    return object != nullptr;
}

void setSpriteShader(string objName, string shaderName)
{
	if (!app->space)
	{
		log("setSpriteShader: GSpace is not available in this scene!");
		return;
	}

	GObject* obj = app->space->getObject(objName);
	if (!obj) {
		log("setSpriteShader: object %s not found!",  objName.c_str());
		return;
	}

    obj->setSpriteShader(shaderName);
}

void showHealth(bool val)
{
    if(!app->hud) throw lua_runtime_error("showHealth: HUD is not available!");
    
    app->hud->showHealth(val);
}

void suppressGameOver(bool b)
{
    app->suppressGameOver = b;
}

void setPaused(bool val)
{
    GScene::crntScene->setPaused(val);
}

unsigned int getFrameNumber()
{
    GSpace* space = app->space;
    if(!space) throw lua_runtime_error("getFrameNumber: Cannot access frame number in this scene.");
    
    return space->getFrame();
}

void dostring_in_inst(string script, string inst_name)
{
    Inst::addCommand(inst_name, script);
}

void castSpellWithArgs(string casterName, string spell, ValueMap args)
{
    if(!app->space) throw lua_runtime_error("castSpellWithArgs: Cannot access objects in this scene.");
    GObject* caster = app->space->getObject(casterName);

    if(!caster){
        throw lua_runtime_error("castSpell: " + casterName + " not found");
    }

    caster->cast(spell, args);
}
    
void castSpell(string casterName, string spell)
{
    castSpellWithArgs(casterName, spell, ValueMap());
}

void stopSpell(string casterName)
{
    if(!app->space) throw lua_runtime_error("stopSpell: Cannot access objects in this scene.");
    GObject* caster = app->space->getObject(casterName);

    if(!caster){
        throw lua_runtime_error("castSpell: " + casterName + " not found");
    }

    caster->stopSpell();
}

bool isObstacle(IntVec2 v)
{
    if(!app->space) throw lua_runtime_error("isObstacle: Cannot access objects in this scene.");

    return app->space->isObstacle(v);
}

void startDialog(string res, bool _auto)
{
    GScene::crntScene->createDialog(res, _auto);
}

void stopDialog()
{
    GScene::crntScene->stopDialog();
}

void setObjectiveCounter(string iconRes, int val)
{
    if(!app->hud) throw lua_runtime_error("setObjectiveCounter: HUD is not available!");
    
    app->hud->objectiveCounter->setVisible(true);
    app->hud->objectiveCounter->setVal(val);
    app->hud->objectiveCounter->setIcon(iconRes);
}

void showObjectiveCounter(bool val)
{
    if(!app->hud) throw lua_runtime_error("setObjectiveCounter: HUD is not available!");

    app->hud->objectiveCounter->setVisible(val);
}

void printGlDebug()
{
    //print the program, vertex, and fragment log for each custom shader.
    
    log("printGlDebug:");
    
    BOOST_FOREACH(string name, App::shaderFiles){
        log("%s", name.c_str());
        GLProgram* p = GLProgramCache::getInstance()->getGLProgram(name);
        
        string logtext;
        
        logtext = p->getProgramLog();
        if(!logtext.empty())
            log("%s program log: \n%s", name.c_str(), logtext.c_str());

        logtext = p->getVertexShaderLog();
        if(!logtext.empty())
            log("%s vertex log: \n%s", name.c_str(), logtext.c_str());

        logtext = p->getFragmentShaderLog();
        if(!logtext.empty())
            log("%s fragment log: \n%s", name.c_str(), logtext.c_str());

    }
}

void save()
{
    GState::save();
}

bool saveCrntReplay(string filepath)
{
	return app->playScene->saveReplayData(filepath);
}

void loadReplay(string sceneName, string replayFilename)
{
	GScene::runSceneWithReplay(sceneName, replayFilename);
}

vector<string> getInventoryContents()
{
    auto registry = GState::crntState.itemRegistry;
    vector<string> items(registry.begin(), registry.end());
    
    return items;
}

vector<pair<float,float>> getPath(pair<int,int> start, pair<int,int> end)
{
    vector<pair<float,float>> result;

    vector<pair<int,int>> tileCoords = graph::gridAStar(
        *app->space->getNavMask(),
        start,
        end,
        app->space->getSize()
    );
    
    //Convert to center position
    foreach(auto tile, tileCoords){
        result.push_back(pair<float,float>(tile.first+0.5, tile.second+0.5));
    }
    
    return result;
}

void runScene(string name)
{
    GScene::runScene(name);
}

void setPlayer(int id)
{
	app->crntPC = static_cast<PlayerCharacter>(id);
}

StateMachineObject* getFSMObject(string funcName,string objName)
{
	if (!app->space)
	{
		log("%s: GSpace is not available in this scene!", funcName.c_str());
		return nullptr;
	}

	GObject* obj = app->space->getObject(objName);
	if (!obj) {
		log("%s: object %s not found!", funcName.c_str(), objName.c_str());
		return nullptr;
	}

	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(obj);
	if (!smo) {
		log("%s: %s is not a StateMachineObject!", funcName.c_str(), objName.c_str());
		return nullptr;
	}

	return smo;
}

shared_ptr<ai::Function> constructState(string funcName, string stateName, GSpace* space, ValueMap args)
{
	shared_ptr<ai::Function> state = ai::Function::constructState(stateName, space, args);
	if (!state) {
		log("%s: Unknown state class %s", funcName.c_str(), stateName.c_str());
		return nullptr;
	}

	return state;
}

void printFSM(string name)
{
    StateMachineObject* smo = getFSMObject("printFMS",name);
    
    if(smo){
        smo->printFSM();
    }
}

unsigned int addThread(string objName, string mainFuncName, ValueMap args)
{
	StateMachineObject* smo = getFSMObject("addThread", objName);

	if (smo) {
		shared_ptr<ai::Function> f = constructState("addThread", mainFuncName, smo->space, args);
		return smo->addThread(f);
	}
	else {
		return 0;
	}
}

void removeThread(string objName, unsigned int uuid)
{
    StateMachineObject* smo = getFSMObject("printFMS",objName);

    if(smo){
        smo->removeThread(uuid);
    }
}

void removeThreadByName(string objName, string threadFuncName)
{
    StateMachineObject* smo = getFSMObject("printFMS",objName);

    if(smo){
        smo->removeThread(threadFuncName);
    }
}


///////////////////////////////////////////////////////////////////

#define make_wrapper(name,f) (name, wrap_cfunction(name,f))
#define make_package_wrapper(ns,name) (#name, wrap_cfunction(#name, ns::name))
#define make_wrapper_same(name) (#name, wrap_cfunction(#name,name))

#define make_method_wrapper(cls, name) \
int name ## _wrapper(lua_State* L) \
{ \
    try{ \
        return methodWrapper(#cls "::" #name, &cls::name, L);\
    }catch(runtime_error err){ \
        Lua::error(L, err.what()); \
        return 1; \
    } \
}

#define install_wrapper(name) installFunction(name ## _wrapper, #name);
#define install_method_wrapper(cls,name) installFunction(name ## _wrapper, #cls "_" #name);
#define add_method(cls,name) Class::addMethod(#cls, #name, name ## _wrapper);

make_method_wrapper(GObject,getPos)
make_method_wrapper(GObject,setPos)
make_method_wrapper(GObject,getVel)
make_method_wrapper(GObject,setVel)
make_method_wrapper(GObject, setDirection)
make_method_wrapper(GObject,getUUID)
make_method_wrapper(GObject,getName)
make_method_wrapper(GObject,setScriptVal)
make_method_wrapper(GObject,getScriptVal)
make_method_wrapper(GObject,_callScriptVal)

void Class::makeClasses()
{
    Class::makeClass("GObject");
//    Class::makeClass("Spellcaster");
    
    add_method(GObject,getPos)
    add_method(GObject,setPos)
    add_method(GObject,getVel)
    add_method(GObject,setVel)
    add_method(GObject, setDirection)
    add_method(GObject,getUUID)
    add_method(GObject,getName)
    add_method(GObject,setScriptVal)
    add_method(GObject,getScriptVal)
    add_method(GObject,_callScriptVal)
}

void Inst::installWrappers()
{
    for(auto it = cfunctions.begin(); it != cfunctions.end(); ++it)
    {
        installNameFunction(it->first);
    }
    
    Class::installClasses(state);
}

const unordered_map<string,function<int(lua_State*)>> Inst::cfunctions = boost::assign::map_list_of
make_wrapper_same(createObject)
make_wrapper_same(removeObject)
make_wrapper_same(setpos)
make_wrapper_same(setvel)
make_wrapper_same(getObjectCount)
make_wrapper_same(getUUIDNameMap)
make_wrapper_same(printMap)
make_wrapper_same(addUpdate)
make_wrapper_same(setscreenscale)
make_wrapper_same(setdpiscale)
make_wrapper_same(setFullscreen)
make_wrapper_same(setResolution)
make_wrapper_same(setFramerate)
make_wrapper_same(getObjByName)
make_wrapper_same(getObjectNames)
make_wrapper_same(isValidObject)
make_wrapper_same(setSpriteShader)
make_wrapper_same(runscript)
make_wrapper_same(showHealth)
make_wrapper_same(setPlayer)
make_wrapper_same(suppressGameOver)
make_wrapper_same(setPaused)
make_wrapper_same(getFrameNumber)
make_wrapper_same(dostring_in_inst)
make_wrapper_same(castSpell)
make_wrapper_same(castSpellWithArgs)
make_wrapper_same(stopSpell)
make_wrapper_same(isObstacle)
make_wrapper_same(startDialog)
make_wrapper_same(stopDialog)
make_wrapper_same(setObjectiveCounter)
make_wrapper_same(showObjectiveCounter)
make_wrapper_same(printGlDebug)
make_wrapper_same(save)
make_wrapper_same(saveCrntReplay)
make_wrapper_same(loadReplay)
make_wrapper_same(getInventoryContents)
make_wrapper_same(getPath)

make_wrapper_same(runScene)

make_wrapper_same(printFSM)
make_wrapper_same(addThread)
make_wrapper_same(removeThread)
make_wrapper_same(removeThreadByName)

make_wrapper_same(toDirection)
make_wrapper_same(stringToDirection)

make_package_wrapper(ai,applyDesiredVelocity)
make_package_wrapper(ai,seek)
make_package_wrapper(ai,flee)

make_package_wrapper(ai,isFacingTarget)
make_package_wrapper(ai,isFacingTargetsBack)
make_package_wrapper(ai,isLineOfSight)

make_package_wrapper(ai,directionToTarget)
make_package_wrapper(ai,displacementToTarget)
make_package_wrapper(ai,distanceToTarget)
make_package_wrapper(ai,viewAngleToTarget)

;

}
