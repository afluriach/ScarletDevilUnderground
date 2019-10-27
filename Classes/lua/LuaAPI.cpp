//
//  Lua.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AI.hpp"
#include "AIFunctions.hpp"
#include "AIMixins.hpp"
#include "AIUtil.hpp"
#include "App.h"
#include "app_constants.hpp"
#include "FileIO.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "GScene.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "PlayScene.hpp"

//Copied from ltm.h.
//#define ttypename(x)	luaT_typenames_[(x) + 1]
//extern const char *const luaT_typenames_[LUA_NUMTAGS+2];

template<typename T, typename... Params>
shared_ptr<ai::Function> create(ai::StateMachine* fsm, Params... params)
{
	return make_shared<T>(fsm, params...);
}

namespace Lua{

const vector<string> Inst::luaIncludes = {
	"util",
	"30log-global",
	"serpent",
	"ai"
};

    Inst::Inst(const string& name) : name(name)
    {
		_state.open_libraries(
			sol::lib::base,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table,
			sol::lib::utf8
		);

        installApi();
        loadLibraries();
        
        if(logInst)
            log("Lua Inst created: %s.", name.c_str());        
    }
    
    Inst::~Inst()
    {        
        if(logInst)
            log("Lua Inst closed: %s.", name.c_str());
    }
    
    void Inst::loadLibraries()
    {
		for(auto const& s : luaIncludes){
            runFile("scripts/"+s+".lua");
        }
    }
        
    void Inst::runString(const string& str)
    {
		try {
			_state.script(str);
		}
		catch (sol::error e){
			log("script %s error: %s", name, e.what());
		}
    }
    
    void Inst::runFile(const string& path)
    {
		runString(io::loadTextFile(path));
    }
    
    void Inst::callIfExistsNoReturn(const string& name)
    {
		sol::function f = _state[name];

		if (f) f();
    }
        
    void Inst::callNoReturn(const string& name)
    {
		_state[name]();
    };
    
#define enum_entry(cls, x) { #x, cls::x }

#define newType(x) _state.new_usertype<x>(#x);
#define addFuncSame(v,x) v[#x] = &_cls::x;
#define cFuncSame(v,x) v[#x] = &x;
#define cFuncSameNS(v,ns,x) v[#x] = &ns::x;

    void Inst::installApi()
    {
		auto app = newType(App);
		#define _cls App

		app["runOverworldScene"] = static_cast<GScene*(*)(string, string)>(&App::runOverworldScene);

		addFuncSame(app,getCrntScene);
		addFuncSame(app, getCrntState);
		addFuncSame(app, printGlDebug);
#if USE_TIMERS
		addFuncSame(app, printTimerInfo);
		addFuncSame(app, setLogTimers);
#endif
		addFuncSame(app, setFullscreen);
		addFuncSame(app, setVsync);
		addFuncSame(app, setMultithread);
		addFuncSame(app, setResolution);
		addFuncSame(app, setFramerate);
		addFuncSame(app, setPlayer);
		addFuncSame(app, setUnlockAllEquips);
		addFuncSame(app, loadProfile);
		addFuncSame(app, saveProfile);

		addFuncSame(app, clearAllKeys);
		addFuncSame(app, clearKeyAction);
		addFuncSame(app, addKeyAction);
#if use_gamepad
		addFuncSame(app, clearAllButtons);
		addFuncSame(app, clearButtonAction);
		addFuncSame(app, addButtonAction);
#endif
		
		auto app_consts = _state.create_table();
		_state["app_constants"] = app_consts;
		
		cFuncSameNS(app_consts, app, baseWidth);
		cFuncSameNS(app_consts, app, baseHeight);
		cFuncSameNS(app_consts, app, pixelsPerTile);
		cFuncSameNS(app_consts, app, tilesPerPixel);
		cFuncSameNS(app_consts, app, viewWidth);
		cFuncSameNS(app_consts, app, Gaccel);

		auto attr = newType(AttributeSystem);
		#define _cls AttributeSystem

		attr["getByName"] = &AttributeSystem::get;
		attr["setByName"] = static_cast<void(AttributeSystem::*)(string, float)>(&AttributeSystem::set);
		attr["getByID"] = &AttributeSystem::operator[];
		attr["setByID"] = &AttributeSystem::_set;
		attr["modifyAttribute"] = static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::modifyAttribute);

		addFuncSame(attr, setFullHP);
		addFuncSame(attr, setFullMP);
		addFuncSame(attr, setFullStamina);

		auto gobject = newType(GObject);
		#define _cls GObject

		addFuncSame(gobject, addGraphicsAction);
		addFuncSame(gobject, stopGraphicsAction);
		addFuncSame(gobject, cast);
		addFuncSame(gobject, getAngle);
		addFuncSame(gobject, getAngularVel);
		addFuncSame(gobject, getPos);
		addFuncSame(gobject, getVel);
		addFuncSame(gobject, setAngle);
		addFuncSame(gobject, setAngularVel);
		addFuncSame(gobject, setPos);
		addFuncSame(gobject, setVel);
		addFuncSame(gobject, setSpriteShader);
		addFuncSame(gobject, setVel);
		addFuncSame(gobject, stopSpell);
		addFuncSame(gobject, printFSM);
			
		gobject["removeThreadByName"] = static_cast<void(GObject::*)(const string&)>(&GObject::removeThread);

		auto agent = _state.new_usertype<Agent>("Agent", sol::base_classes, sol::bases<GObject>());
		#define _cls Agent

		addFuncSame(agent, getAttributeSystem);

		auto gscene = newType(GScene);
		#define _cls GScene

		addFuncSame(gscene, runScene);
		addFuncSame(gscene, runSceneWithReplay);
		addFuncSame(gscene, getSpace);
		addFuncSame(gscene, setPaused);
		addFuncSame(gscene, stopDialog);
		addFuncSame(gscene, teleportToDoor);
		addFuncSame(gscene, setRoomVisible);
		addFuncSame(gscene, unlockAllRooms);
		
		gscene["createDialog"] = static_cast<void(GScene::*)(const string&, bool)>(&GScene::createDialog);

		auto playscene = _state.new_usertype<PlayScene>("PlayScene", sol::base_classes, sol::bases<GScene>());
		#define _cls PlayScene
		
		addFuncSame(playscene, saveReplayData);

		auto gspace = newType(GSpace);
		#define _cls GSpace

		gspace["createObject"] = static_cast<gobject_ref(GSpace::*)(const ValueMap&)>(&GSpace::createObject);
		gspace["getObjectByName"] = static_cast<GObject*(GSpace::*)(const string&) const>(&GSpace::getObject);
		addFuncSame(gspace, getFrame);
		addFuncSame(gspace, getObjectCount);
		addFuncSame(gspace, getObjectNames);
		addFuncSame(gspace, getUUIDNameMap);
		addFuncSame(gspace, isObstacle);
		gspace["removeObject"] = static_cast<void(GSpace::*)(const string&)>(&GSpace::removeObject);
			
		auto gstate = newType(GState);
		#define _cls GState

		addFuncSame(gstate, addItem);
		addFuncSame(gstate, hasItem);
		addFuncSame(gstate, _registerChamberCompleted);
		addFuncSame(gstate, registerUpgrade);
		addFuncSame(gstate, setUpgradeLevels);

		auto hud = newType(HUD);
		#define _cls HUD

		addFuncSame(hud, setMansionMode);
		addFuncSame(hud, setObjectiveCounter);
		addFuncSame(hud, setObjectiveCounterVisible);

		auto graphics = _state.create_table();
		_state["graphics"] = graphics;

		cFuncSame(graphics, indefiniteFlickerAction);
		cFuncSame(graphics, flickerAction);
		cFuncSame(graphics, flickerTintAction);
		cFuncSame(graphics, comboFlickerTintAction);
		cFuncSame(graphics, spellcardFlickerTintAction);
		cFuncSame(graphics, darknessCurseFlickerTintAction);
		cFuncSame(graphics, tintToAction);
		cFuncSame(graphics, motionBlurStretch);
		cFuncSame(graphics, bombAnimationAction);
		cFuncSame(graphics, freezeEffectAction);
		cFuncSame(graphics, freezeEffectEndAction);
		cFuncSame(graphics, objectFadeOut);
		cFuncSame(graphics, damageIndicatorAction);

		auto _ai = _state.create_table();
		_state["ai"] = _ai;

		auto _event_type = _state.new_enum<ai::event_type, true>(
			"event_type",
			{
				enum_entry(ai::event_type, begin),
				enum_entry(ai::event_type, bulletBlock),
				enum_entry(ai::event_type, bulletHit),
				enum_entry(ai::event_type, detect),
				enum_entry(ai::event_type, endDetect),
				enum_entry(ai::event_type, zeroHP),
				enum_entry(ai::event_type, zeroStamina),
				enum_entry(ai::event_type, end),
			}
		);

		_ai["event_type"] = _event_type;

		auto event = _state.new_usertype<ai::Event>("Event");
		#define _cls ai::Event
		_ai["Event"] = event;
		
		addFuncSame(event, isDetectPlayer);
		addFuncSame(event, getDetectType);
		addFuncSame(event, getEndDetectType);
		addFuncSame(event, getEventType);

		#define _cls ai::Function
		auto func = _state.new_usertype<ai::Function>("Function");
		_ai["Function"] = func;

		addFuncSame(func, getSpace);
		addFuncSame(func, getAgent);
		addFuncSame(func, onEnter);
		addFuncSame(func, update);
		addFuncSame(func, onReturn);
		addFuncSame(func, onExit);
		addFuncSame(func, onEvent);
		addFuncSame(func, getEvents);
		addFuncSame(func, getName);

		#define _cls ai::Thread
		auto thread = _state.new_usertype<ai::Thread>(
			"Thread",
			sol::constructors < ai::Thread(shared_ptr < ai::Function>, ai::StateMachine*) > ()
		);
		_ai["Thread"] = thread;

		addFuncSame(thread, update);
		addFuncSame(thread, onEvent);
		addFuncSame(thread, push);
		addFuncSame(thread, pop);
		addFuncSame(thread, popToRoot);
		addFuncSame(thread, getTop);
		addFuncSame(thread, getStack);
		addFuncSame(thread, getMainFuncName);

		#define _cls ai::StateMachine
		auto sm = _state.new_usertype<ai::StateMachine>("StateMachine");
		_state["ai"]["StateMachine"] = sm;

		sm["addFunction"] = static_cast<void(ai::StateMachine::*)(shared_ptr<ai::Function>)>(&ai::StateMachine::addFunction);
		addFuncSame(sm, removeFunction);
		sm["addThread"] = sol::overload(
			static_cast<shared_ptr<ai::Thread>(ai::StateMachine::*)(shared_ptr<ai::Function>)>(&ai::StateMachine::addThread),
			static_cast<shared_ptr<ai::Thread>(ai::StateMachine::*)(shared_ptr<ai::Thread>)>(&ai::StateMachine::addThread)
		);
		sm["removeThread"] = sol::overload(
			static_cast<void(ai::StateMachine::*)(shared_ptr<ai::Thread>)>(&ai::StateMachine::removeThread),
			static_cast<void(ai::StateMachine::*)(const string&)>(&ai::StateMachine::removeThread)
		);
		addFuncSame(sm, isThreadRunning);
		addFuncSame(sm, getThreadCount);

		addFuncSame(sm, addFleeBomb);

		addFuncSame(sm, getSpace);
		addFuncSame(sm, getObject);
		addFuncSame(sm, getAgent);
		addFuncSame(sm, getFrame);
		addFuncSame(sm, toString);

		auto flock = _state.new_usertype<ai::Flock>(
			"Flock",
			sol::base_classes, sol::bases<ai::Function>()
		);
		_state["ai"]["Flock"] = flock;
		_state["ai"]["Flock"]["create"] = &create<ai::Flock>;

		auto idle = _state.new_usertype<ai::IdleWait>(
			"IdleWait",
			sol::base_classes, sol::bases<ai::Function>()
		);
		_state["ai"]["IdleWait"] = idle;
		_state["ai"]["IdleWait"]["create"] = &create<ai::IdleWait>;

		auto wander = _state.new_usertype<ai::Wander>(
			"Wander",
			sol::base_classes, sol::bases<ai::Function>()
			);
		_state["ai"]["Wander"] = wander;
		_state["ai"]["Wander"]["create"] = &create<ai::Wander>;

	}
}
