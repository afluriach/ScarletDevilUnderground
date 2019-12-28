//
//  Lua.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "app_constants.hpp"
#include "FileIO.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "GScene.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "xml.hpp"

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
    
    void Inst::installApi()
    {
		auto app = newType(App);
		#define _cls App

		app["runOverworldScene"] = static_cast<GScene*(*)(string, string)>(&App::runOverworldScene);

		addFuncSame(app, getParams);
		addFuncSame(app,getCrntScene);
		addFuncSame(app, getCrntState);
		addFuncSame(app, printGlDebug);
#if USE_TIMERS
		addFuncSame(app, printTimerInfo);
		addFuncSame(app, setLogTimers);
#endif
		addFuncSame(app, setFullscreen);
		addFuncSame(app, setVsync);
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
		
		app["getBullet"] = &app::getBullet;

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

		attr["get"] = sol::overload(
			&AttributeSystem::get,
			&AttributeSystem::operator[]
		);

		attr["set"] = sol::overload(
			static_cast<void(AttributeSystem::*)(string, float)>(&AttributeSystem::set),
			static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::set)
		);

		attr["modifyAttribute"] = static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::modifyAttribute);

		addFuncSame(attr, isZero);
		addFuncSame(attr, isNonzero);
		addFuncSame(attr, hasHitProtection);
		addFuncSame(attr, setTimedProtection);
		addFuncSame(attr, timerDecrement);
		addFuncSame(attr, timerIncrement);
		addFuncSame(attr, setFullHP);
		addFuncSame(attr, setFullMP);
		addFuncSame(attr, setFullStamina);

		auto objref = newType(gobject_ref);
		objref["isValid"] = &gobject_ref::isValid;
		objref["isFuture"] = &gobject_ref::isFuture;
		objref["get"] = &gobject_ref::get;
		objref["getID"] = &gobject_ref::getID;

		auto objparams = _state.new_usertype<object_params>(
			"object_params",
			sol::constructors<
				object_params()
			>()
		);

		#define _cls bullet_properties
		auto bullet_props = _state.new_usertype<bullet_properties>("bullet_properties");
		addFuncSame(bullet_props, speed);
		addFuncSame(bullet_props, dimensions);

		addFuncSame(bullet_props, damage);

		addFuncSame(bullet_props, sprite);
		addFuncSame(bullet_props, lightSource);

		addFuncSame(bullet_props, hitCount);
		addFuncSame(bullet_props, ricochetCount);
		addFuncSame(bullet_props, directionalLaunch);
		addFuncSame(bullet_props, ignoreObstacles);
		addFuncSame(bullet_props, deflectBullets);

		bullet_props["clone"] = &bullet_properties::clone;

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
		addFuncSame(gspace, createBullet);
		addFuncSame(gspace, getPlayerAsRef);
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

		auto action_tags = _state.new_enum<cocos_action_tag, true>(
			"cocos_action_tag",
			{
				enum_entry(cocos_action_tag, illusion_dash),
				enum_entry(cocos_action_tag, damage_flicker),
				enum_entry(cocos_action_tag, object_fade),
				enum_entry(cocos_action_tag, hit_protection_flicker),
				enum_entry(cocos_action_tag, combo_mode_flicker),
				enum_entry(cocos_action_tag, freeze_status),
				enum_entry(cocos_action_tag, darkness_curse),
				enum_entry(cocos_action_tag, game_over_tint),
			}
		);

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

		addAI();
		addGObject();
		addMagic();
		addTypes();
	}
}
