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
#include "GSpace.hpp"
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
    
#define enum_entry(cls, x) { #x, cls::x }

#define newType(x) _state.new_usertype<x>(#x);
#define addFuncSame(v,x) v[#x] = &_cls::x;
#define cFuncSame(v,x) v[#x] = &x;
#define cFuncSameNS(v,ns,x) v[#x] = &ns::x;

    void Inst::installApi()
    {
		auto gtype = _state.new_enum<GType, true>(
			"GType",
			{
				enum_entry(GType, none),
				enum_entry(GType, player),
				enum_entry(GType, playerBullet),
				enum_entry(GType, enemy),
				enum_entry(GType, enemyBullet),
				enum_entry(GType, environment),
				enum_entry(GType, foliage),
				enum_entry(GType, wall),
				enum_entry(GType, areaSensor),
				enum_entry(GType, enemySensor),
				enum_entry(GType, playerGrazeRadar),
				enum_entry(GType, playerPickup),
				enum_entry(GType, npc),
				enum_entry(GType, floorSegment),
				enum_entry(GType, bomb),
				enum_entry(GType, all),
			}
		);

		auto damageType = _state.new_enum <DamageType, true>(
			"DamageType",
			{
				enum_entry(DamageType, bullet),
				enum_entry(DamageType, bomb),
				enum_entry(DamageType, effectArea),
				enum_entry(DamageType, touch),
				enum_entry(DamageType, melee),
				enum_entry(DamageType, pitfall),
			}
		);

		auto _attr = _state.new_enum <Attribute, true>(
			"Attribute",
			{
				enum_entry(Attribute, hp),
				enum_entry(Attribute, mp),
				enum_entry(Attribute, stamina),
				enum_entry(Attribute, hitProtection),
				enum_entry(Attribute, spellCooldown),

				enum_entry(Attribute, maxHP),
				enum_entry(Attribute, maxMP),
				enum_entry(Attribute, maxStamina),
				enum_entry(Attribute, hitProtectionInterval),
				enum_entry(Attribute, spellCooldownInterval),

				enum_entry(Attribute, hpRegen),
				enum_entry(Attribute, mpRegen),
				enum_entry(Attribute, staminaRegen),

				enum_entry(Attribute, keys),
				enum_entry(Attribute, combo),
				enum_entry(Attribute, touchDamage),

				enum_entry(Attribute, attack),
				enum_entry(Attribute, attackSpeed),
				enum_entry(Attribute, bulletSpeed),

				enum_entry(Attribute, shieldLevel),

				enum_entry(Attribute, stress),
				enum_entry(Attribute, stressDecay),
				enum_entry(Attribute, stressFromHits),
				enum_entry(Attribute, stressFromBlocks),
				enum_entry(Attribute, stressFromDetects),

				enum_entry(Attribute, agility),
				enum_entry(Attribute, speed),
				enum_entry(Attribute, acceleration),

				enum_entry(Attribute, bombSensitivity),
				enum_entry(Attribute, bulletSensitivity),
				enum_entry(Attribute, meleeSensitivity),

				enum_entry(Attribute, iceSensitivity),
				enum_entry(Attribute, sunSensitivity),
				enum_entry(Attribute, darknessSensitivity),
				enum_entry(Attribute, poisonSensitivity),
				enum_entry(Attribute, slimeSensitivity),

				enum_entry(Attribute, iceDamage),
				enum_entry(Attribute, sunDamage),
				enum_entry(Attribute, darknessDamage),
				enum_entry(Attribute, poisonDamage),
				enum_entry(Attribute, slimeDamage),
			}
		);

		auto damageInfo = _state.new_usertype<DamageInfo>(
			"DamageInfo",
			sol::constructors< DamageInfo(), DamageInfo(float,DamageType), DamageInfo(float,Attribute,DamageType) >()
		);

		damageInfo["mag"] = &DamageInfo::mag;
		damageInfo["element"] = &DamageInfo::element;
		damageInfo["type"] = &DamageInfo::type;
		damageInfo["scale"] = &DamageInfo::operator*;

		auto vect = _state.new_usertype<SpaceVect>(
			"SpaceVect",
			sol::constructors<SpaceVect()>()
		);

		vect["ray"] = &SpaceVect::ray;

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

		attr["getByName"] = &AttributeSystem::get;
		attr["setByName"] = static_cast<void(AttributeSystem::*)(string, float)>(&AttributeSystem::set);
		attr["getByID"] = &AttributeSystem::operator[];
		attr["setByID"] = &AttributeSystem::_set;
		attr["modifyAttribute"] = static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::modifyAttribute);

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

		auto gobject = newType(GObject);
		#define _cls GObject

		addFuncSame(gobject, addGraphicsAction);
		addFuncSame(gobject, stopGraphicsAction);
		addFuncSame(gobject, cast);
		addFuncSame(gobject, getMaxSpeed);
		addFuncSame(gobject, getAngle);
		addFuncSame(gobject, getAngularVel);
		addFuncSame(gobject, getPos);
		addFuncSame(gobject, getVel);
		addFuncSame(gobject, rotate);
		addFuncSame(gobject, setAngle);
		addFuncSame(gobject, setAngularVel);
		addFuncSame(gobject, setPos);
		addFuncSame(gobject, setVel);
		addFuncSame(gobject, setSpriteShader);
		addFuncSame(gobject, setVel);
		addFuncSame(gobject, setFrozen);
		addFuncSame(gobject, stopSpell);
		addFuncSame(gobject, printFSM);
			
		gobject["removeThreadByName"] = static_cast<void(GObject::*)(const string&)>(&GObject::removeThread);

		auto agent = _state.new_usertype<Agent>("Agent", sol::base_classes, sol::bases<GObject>());
		#define _cls Agent

		addFuncSame(agent, getAttributeSystem);
		addFuncSame(agent, getBulletAttributes);

		addFuncSame(agent, isFiringSuppressed);
		addFuncSame(agent, setFiringSuppressed);
		addFuncSame(agent, isMovementSuppressed);
		addFuncSame(agent, setMovementSuppressed);

		auto player = _state.new_usertype<Player>("Player", sol::base_classes, sol::bases<Agent>());

		#define _cls bullet_properties
		auto bullet_props = _state.new_usertype<bullet_properties>("bullet_properties");
		addFuncSame(bullet_props, mass);
		addFuncSame(bullet_props, speed);
		addFuncSame(bullet_props, dimensions);
		addFuncSame(bullet_props, knockback);

		addFuncSame(bullet_props, damage);

		addFuncSame(bullet_props, sprite);
		addFuncSame(bullet_props, lightSource);

		addFuncSame(bullet_props, hitCount);
		addFuncSame(bullet_props, ricochetCount);
		addFuncSame(bullet_props, directionalLaunch);
		addFuncSame(bullet_props, ignoreObstacles);
		addFuncSame(bullet_props, deflectBullets);

		bullet_props["clone"] = &bullet_properties::clone;

		auto bullet = _state.new_usertype<Bullet>("Bullet", sol::base_classes, sol::bases<GObject>());
		bullet["makeParams"] = &Bullet::makeParams;

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
	}
}
