//
//  lua_gobject.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "EnvironmentObject.hpp"
#include "FloorSegment.hpp"
#include "graphics_types.h"
#include "LuaAPI.hpp"
#include "Item.hpp"
#include "MagicEffect.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "SpellDescriptor.hpp"
#include "Wall.hpp"

namespace Lua{
    
    void Inst::addGObject()
    {
		auto objects = _state.create_table();
		_state["objects"] = objects;

		auto objref = _state.new_usertype<gobject_ref>(
			"gobject_ref",
			sol::constructors<
				gobject_ref(),
				gobject_ref(const gobject_ref&),
				gobject_ref(const GObject*)
			>(),
			sol::meta_function::equal_to, &gobject_ref::operator==,
			"id", sol::property(&gobject_ref::getID)
		);
		objref["isValid"] = &gobject_ref::isValid;
		objref["isFuture"] = &gobject_ref::isFuture;
		objref["get"] = &gobject_ref::get;

		auto objparams = _state.new_usertype<object_params>(
			"object_params",
			sol::constructors<
				object_params()
			>(),
			rw_prop(object_params, pos),
			rw_prop(object_params, angle),
			rw_prop(object_params, vel),
			rw_prop(object_params, angularVel),
			rw_prop(object_params, dimensions),
			rw_prop(object_params, name),
			get_prop(object_params, args),
			rw_prop(object_params, hidden),
			rw_prop(object_params, active)
		);

#define _cls object_properties
		auto object_props = _state.new_usertype<object_properties>(
			"object_properties",
			rw_prop(object_properties, light),
			rw_prop(object_properties, sprite),
			rw_prop(object_properties, properName),
			rw_prop(object_properties, clsName),
			rw_prop(object_properties, dimensions),
			rw_prop(object_properties, mass),
			rw_prop(object_properties, friction)
		);

		addFuncSame(object_props, getLightColor);

		auto agent_props = _state.new_usertype<agent_properties>(
			"agent_properties",
			sol::base_classes, sol::bases<object_properties>()
		);

#define _cls bullet_properties
		auto bullet_props = _state.new_usertype<bullet_properties>(
			"bullet_properties",
			sol::base_classes, sol::bases<object_properties>(),
			rw_prop(bullet_properties, speed),
			rw_prop(bullet_properties, damage),
			rw_prop(bullet_properties, hitCount),
			rw_prop(bullet_properties, ricochetCount),
			rw_prop(bullet_properties, invisible),
			rw_prop(bullet_properties, ignoreObstacles),
			rw_prop(bullet_properties, deflectBullets)
		);

#define _cls bullet_attributes
		auto bullet_attr = _state.new_usertype<bullet_attributes>(
			"bullet_attributes",
			rw_prop(bullet_attributes, casterVelocity),
			rw_prop(bullet_attributes, caster),
			rw_prop(bullet_attributes, type),
			rw_prop(bullet_attributes, sourceSpell),
			rw_prop(bullet_attributes, size),
			rw_prop(bullet_attributes, attackDamage),
			rw_prop(bullet_attributes, bulletSpeed)
		);
		addFuncSame(bullet_attr, getDefault);

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

		#define _cls GObject
		auto gobject = _state.new_usertype<GObject>(
			"GObject",
			"active", sol::property(&GObject::getActive),
			"pos", sol::property(&GObject::getPos, &GObject::setPos),
			"dim", sol::property(&GObject::getDimensions),
			"level", sol::property(&GObject::getLevel),
			"name", sol::property(&GObject::getName),
			"id", sol::property(&GObject::getUUID),
            "props", sol::property(&GObject::getProps),
			"space", sol::property(&GObject::getSpace)
		);

		addFuncSame(gobject, sprite);
		addFuncSame(gobject, light);
		addFuncSame(gobject, drawNode);

		addFuncSame(gobject, playSoundSpatial);

		addFuncSame(gobject, teleport);
		addFuncSame(gobject, cast);
		addFuncSame(gobject, hit);
		addFuncSame(gobject, applyMagicEffect);
		addFuncSame(gobject, getMaxSpeed);
		addFuncSame(gobject, getAngle);
		addFuncSame(gobject, getAngularVel);
		addFuncSame(gobject, getPos);
		addFuncSame(gobject, getVel);
		addFuncSame(gobject, rotate);
        addFuncSame(gobject, setDirection);
		addFuncSame(gobject, setAngle);
		addFuncSame(gobject, setAngularVel);
		addFuncSame(gobject, setPos);
		addFuncSame(gobject, setVel);
        addFuncSame(gobject, getInitialCenterPix);

		addFuncSame(gobject, createLight);
		addFuncSame(gobject, removeLight);
		addFuncSame(gobject, createDrawNode);

		addFuncSame(gobject, setBodySensor);
		addFuncSame(gobject, getBodySensor);
		addFuncSame(gobject, launchAtTarget);
		addFuncSame(gobject, getIsOnFloor);
		addFuncSame(gobject, setIsOnFloor);
		addFuncSame(gobject, activate);
		addFuncSame(gobject, deactivate);
		addFuncSame(gobject, getActive);
		addFuncSame(gobject, toggleActive);

		addFuncSame(gobject, getBulletAttributes);
		addFuncSame(gobject, launchBullet);
		addFuncSame(gobject, spawnBullet);
		addFuncSame(gobject, getSpace);
		addFuncSame(gobject, toString);
		addFuncSame(gobject, getName);
		addFuncSame(gobject, getClsName);
		addFuncSame(gobject, getType);
		addFuncSame(gobject, getFullType);
			
		gobject["getAsObject"] = &GObject::getAs<GObject>;
		gobject["getRef"] = &GObject::getRef;
		gobject["getAsAgent"] = &GObject::getAs<Agent>;
		gobject["getAsBullet"] = &GObject::getAs<Bullet>;
		gobject["getAsEnvironmentObject"] = &GObject::getAs<EnvironmentObject>;
		gobject["getAsItem"] = &GObject::getAs<Item>;
		gobject["getAsPlayer"] = &GObject::getAs<Player>;

		auto agent = _state.new_usertype<Agent>("Agent", sol::base_classes, sol::bases<GObject>());
		#define _cls Agent

		addFuncSame(agent, setSprite);
		
		addFuncSame(agent, hit);
		addFuncSame(agent, getLevel);
		addFuncSame(agent, get);
		addFuncSame(agent, increment);
		addFuncSame(agent, decrement);
		addFuncSame(agent, isActive);
		addFuncSame(agent, getAttributeSystem);

		addFuncSame(agent, getSensedObjectDistance);
        
        addFuncSame(agent, aimAtTarget);
        addFuncSame(agent, fire);

		agent["modifyAttribute"] = sol::overload(
			static_cast< void(Agent::*)(Attribute,float) >(&Agent::modifyAttribute),
			static_cast< void(Agent::*)(Attribute, Attribute) >(&Agent::modifyAttribute),
			static_cast< void(Agent::*)(Attribute, Attribute, float) >(&Agent::modifyAttribute)
		);

		auto npc = _state.new_usertype<NPC>("NPC", sol::base_classes, sol::bases<GObject, Agent>());

		auto enemy = _state.new_usertype<Enemy>("Enemy", sol::base_classes, sol::bases<GObject, Agent>());

		auto player = _state.new_usertype<Player>("Player", sol::base_classes, sol::bases<GObject, Agent>());
#define _cls Player

		addFuncSame(player, applyUpgrade);
		addFuncSame(player, equipItems);
        addFuncSame(player, gameOver);

		auto bullet = _state.new_usertype<Bullet>(
			"Bullet",
			sol::base_classes, sol::bases<GObject>()
		);
		bullet["makeParams"] = &Bullet::makeParams;

		auto environment = _state.new_usertype<EnvironmentObject>(
			"EnvironmentObject",
			sol::base_classes, sol::bases<GObject>()
		);

		auto floor = _state.new_usertype<FloorSegment>(
			"FloorSegment",
			sol::base_classes, sol::bases<GObject>()
		);

#define _cls Item
		auto item = _state.new_usertype<Item>(
			"Item",
			sol::base_classes, sol::bases<GObject>()
		);
		
		auto wall = _state.new_usertype<Wall>(
			"Wall",
			sol::base_classes, sol::bases<GObject>()
		);
	}

}
