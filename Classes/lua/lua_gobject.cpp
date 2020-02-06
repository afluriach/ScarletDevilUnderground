//
//  lua_gobject.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Bullet.hpp"
#include "EnvironmentObject.hpp"
#include "LuaAPI.hpp"
#include "Item.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "Torch.hpp"

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
			>()
		);
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

#define _cls object_properties
		auto object_props = _state.new_usertype<object_properties>("object_properties");

		auto agent_props = _state.new_usertype<agent_properties>(
			"agent_properties",
			sol::base_classes, sol::bases<object_properties>()
		);

		auto agent_attrs = _state.new_usertype<agent_attributes>(
			"agent_attributes",

			"name", sol::property(&agent_attributes::getName),
			"level", sol::property(&agent_attributes::getLevel)
		);

#define _cls bullet_properties
		auto bullet_props = _state.new_usertype<bullet_properties>(
			"bullet_properties",
			sol::base_classes, sol::bases<object_properties>()
		);

		addFuncSame(bullet_props, speed);

		addFuncSame(bullet_props, damage);

		addFuncSame(bullet_props, hitCount);
		addFuncSame(bullet_props, ricochetCount);
		addFuncSame(bullet_props, directionalLaunch);
		addFuncSame(bullet_props, ignoreObstacles);
		addFuncSame(bullet_props, deflectBullets);

		bullet_props["clone"] = &bullet_properties::clone;

#define _cls bullet_attributes
		auto bullet_attr = _state.new_usertype<bullet_attributes>("bullet_attributes");
		addFuncSame(bullet_attr, getDefault);
		addFuncSame(bullet_attr, casterVelocity);
		addFuncSame(bullet_attr, caster);
		addFuncSame(bullet_attr, type);
		addFuncSame(bullet_attr, size);
		addFuncSame(bullet_attr, attackDamage);
		addFuncSame(bullet_attr, bulletSpeed);

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

		auto gobject = _state.new_usertype<GObject>(
			"GObject",
			"space", sol::property(&GObject::getSpace)
		);

		#define _cls GObject

		gobject["addGraphicsAction"] = sol::overload(
			static_cast<void(GObject::*)(GraphicsAction)>(&GObject::addGraphicsAction),
			static_cast<void(GObject::*)(GraphicsAction, SpriteID)>(&GObject::addGraphicsAction)
		);
		gobject["stopGraphicsAction"] = sol::overload(
			static_cast<void(GObject::*)(cocos_action_tag)>(&GObject::stopGraphicsAction),
			static_cast<void(GObject::*)(cocos_action_tag, SpriteID)>(&GObject::stopGraphicsAction)
		);

		addFuncSame(gobject, setSpriteZoom);
		addFuncSame(gobject, cast);
		addFuncSame(gobject, hit);
		addFuncSame(gobject, applyMagicEffect);
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
		addFuncSame(gobject, setSpriteOpacity);
		addFuncSame(gobject, setVel);
		addFuncSame(gobject, launchAtTarget);
		addFuncSame(gobject, setLayers);
		addFuncSame(gobject, setFrozen);
		addFuncSame(gobject, setInhibitSpellcasting);
		addFuncSame(gobject, setInvisible);

		addFuncSame(gobject, getBulletAttributes);
		addFuncSame(gobject, spawnBullet);
		addFuncSame(gobject, getSpace);
		addFuncSame(gobject, getName);
		addFuncSame(gobject, getClsName);

		gobject["launchBullet"] = sol::overload(
			[](GObject* obj, local_shared_ptr<bullet_properties> props,SpaceVect displacement,SpaceFloat angle) -> gobject_ref {
				return obj->launchBullet(props, displacement, angle);
			},
			[](GObject* obj, local_shared_ptr<bullet_properties> props, SpaceVect displacement, SpaceFloat angle, SpaceFloat angularVelocity) -> gobject_ref {
				return obj->launchBullet(props, displacement, angle, angularVelocity);
			},
			[](GObject* obj, local_shared_ptr<bullet_properties> props, SpaceVect displacement, SpaceFloat angle, SpaceFloat angularVelocity, bool obstacleCheck) -> gobject_ref {
				return obj->launchBullet(props, displacement, angle, angularVelocity, obstacleCheck);
			}
		);

		addFuncSame(gobject, printFSM);
			
		gobject["asGObject"] = &GObject::getAs<GObject>;
		gobject["getRef"] = [](const GObject* obj) -> gobject_ref { return gobject_ref(obj); };
		gobject["getAsAgent"] = &GObject::getAs<Agent>;
		gobject["getAsBullet"] = &GObject::getAs<Bullet>;
		gobject["getAsPlayer"] = &GObject::getAs<Player>;
		gobject["getAsTorch"] = &GObject::getAs<Torch>;
		gobject["removeThreadByName"] = static_cast<void(GObject::*)(const string&)>(&GObject::removeThread);

		auto agent = _state.new_usertype<Agent>("Agent", sol::base_classes, sol::bases<GObject>());
		#define _cls Agent

		addFuncSame(agent, setSprite);
		
		addFuncSame(agent, hit);
		addFuncSame(agent, getLevel);
		addFuncSame(agent, getAttribute);
		addFuncSame(agent, getAttributeSystem);

		addFuncSame(agent, modifyAttribute);

		addFuncSame(agent, setProtection);
		addFuncSame(agent, setTimedProtection);
		addFuncSame(agent, resetProtection);

		addFuncSame(agent, isFiringSuppressed);
		addFuncSame(agent, setFiringSuppressed);
		addFuncSame(agent, isMovementSuppressed);
		addFuncSame(agent, setMovementSuppressed);

		auto npc = _state.new_usertype<NPC>("NPC", sol::base_classes, sol::bases<GObject, Agent>());
#define _cls NPC

		auto player = _state.new_usertype<Player>("Player", sol::base_classes, sol::bases<GObject, Agent>());
#define _cls Player

		addFuncSame(player, applyUpgrade);
		addFuncSame(player, equipItems);

		auto bullet = _state.new_usertype<Bullet>("Bullet", sol::base_classes, sol::bases<GObject>());
		bullet["makeParams"] = sol::overload(
			[](SpaceVect pos, SpaceFloat angle)->local_shared_ptr<object_params> { return Bullet::makeParams(pos, angle); },
			[](SpaceVect pos, SpaceFloat angle, SpaceVect vel)->local_shared_ptr<object_params> { return Bullet::makeParams(pos, angle, vel); },
			[](SpaceVect pos, SpaceFloat angle, SpaceVect vel, SpaceFloat angularVel)->local_shared_ptr<object_params> { return Bullet::makeParams(pos, angle, vel,angularVel); }
		);

		auto environment = _state.new_usertype<EnvironmentObject>(
			"EnvironmentObject",
			sol::base_classes, sol::bases<GObject>()
		);

		auto torch = _state.new_usertype<Torch>("Torch", sol::base_classes, sol::bases <GObject>());
#define _cls Torch
		addFuncSame(torch, getActive);
		addFuncSame(torch, setActive);
		addFuncSame(torch, hit);

#define _cls Item
		auto item = _state.new_usertype<Item>(
			"Item",
			sol::base_classes, sol::bases<GObject>()
		);
	}

}
