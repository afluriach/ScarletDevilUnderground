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
#include "MagicEffect.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "SpellDescriptor.hpp"
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

#define _cls bullet_attributes
		auto bullet_attr = _state.new_usertype<bullet_attributes>(
			"bullet_attributes",
			"casterVelocity", sol::property(&bullet_attributes::getCasterVel, &bullet_attributes::setCasterVel),
			"caster", sol::property(&bullet_attributes::getCaster, &bullet_attributes::setCaster),
			"type", sol::property(&bullet_attributes::getType, &bullet_attributes::setType),
			"spell", sol::property(&bullet_attributes::getSourceSpell, &bullet_attributes::setSourceSpell),
			"size", sol::property(&bullet_attributes::getSize, &bullet_attributes::setSize),
			"attackDamage", sol::property(&bullet_attributes::getAttackDamage, &bullet_attributes::setAttackDamage),
			"bulletSpeed", sol::property(&bullet_attributes::getBulletSpeed, &bullet_attributes::setBulletSpeed)
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

		auto gobject = _state.new_usertype<GObject>(
			"GObject",
			"active", sol::property(&EnvironmentObject::getActive),
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

		addFuncSame(gobject, playSoundSpatial);
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
		addFuncSame(gobject, setSpriteVisible);
		addFuncSame(gobject, setSpriteTexture);
		addFuncSame(gobject, setBodySensor);
		addFuncSame(gobject, getBodySensor);
		addFuncSame(gobject, launchAtTarget);
		addFuncSame(gobject, setLayers);
		addFuncSame(gobject, setFrozen);
		addFuncSame(gobject, activate);
		addFuncSame(gobject, deactivate);

		addFuncSame(gobject, getBulletAttributes);
		addFuncSame(gobject, spawnBullet);
		addFuncSame(gobject, getSpace);
		addFuncSame(gobject, getName);
		addFuncSame(gobject, getClsName);
		addFuncSame(gobject, getType);
		addFuncSame(gobject, getFullType);

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
		addFuncSame(agent, get);
		addFuncSame(agent, getAttributeSystem);

		agent["modifyAttribute"] = sol::overload(
			static_cast< void(Agent::*)(Attribute,float) >(&Agent::modifyAttribute),
			static_cast< void(Agent::*)(Attribute, Attribute) >(&Agent::modifyAttribute),
			static_cast< void(Agent::*)(Attribute, Attribute, float) >(&Agent::modifyAttribute)
		);

		addFuncSame(agent, increment);
		addFuncSame(agent, decrement);
		addFuncSame(agent, isActive);

		auto npc = _state.new_usertype<NPC>("NPC", sol::base_classes, sol::bases<GObject, Agent>());
#define _cls NPC

		auto player = _state.new_usertype<Player>("Player", sol::base_classes, sol::bases<GObject, Agent>());
#define _cls Player

		addFuncSame(player, applyUpgrade);
		addFuncSame(player, equipItems);

		auto bullet = _state.new_usertype<Bullet>("Bullet", sol::base_classes, sol::bases<GObject>());
		bullet["makeParams"] = sol::overload(
			[](SpaceVect pos, SpaceFloat angle) -> object_params { return Bullet::makeParams(pos, angle); },
			[](SpaceVect pos, SpaceFloat angle, SpaceVect vel) -> object_params { return Bullet::makeParams(pos, angle, vel); },
			[](SpaceVect pos, SpaceFloat angle, SpaceVect vel, SpaceFloat angularVel) -> object_params { return Bullet::makeParams(pos, angle, vel,angularVel); }
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
