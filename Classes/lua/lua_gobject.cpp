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
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "Torch.hpp"

namespace Lua{
    
    void Inst::addGObject()
    {
		auto gobject = newType(GObject);
		#define _cls GObject

		addFuncSame(gobject, addGraphicsAction);
		addFuncSame(gobject, stopGraphicsAction);
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

		gobject["launchBullet"] = sol::overload(
			[](GObject* obj, shared_ptr<bullet_properties> props,SpaceVect displacement,SpaceFloat angle) -> gobject_ref {
				return obj->launchBullet(props, displacement, angle);
			},
			[](GObject* obj, shared_ptr<bullet_properties> props, SpaceVect displacement, SpaceFloat angle, SpaceFloat angularVelocity) -> gobject_ref {
				return obj->launchBullet(props, displacement, angle, angularVelocity);
			},
			[](GObject* obj, shared_ptr<bullet_properties> props, SpaceVect displacement, SpaceFloat angle, SpaceFloat angularVelocity, bool obstacleCheck) -> gobject_ref {
				return obj->launchBullet(props, displacement, angle, angularVelocity, obstacleCheck);
			}
		);


		addFuncSame(gobject, stopSpell);
		addFuncSame(gobject, printFSM);
			
		gobject["getAsAgent"] = &GObject::getAs<Agent>;
		gobject["removeThreadByName"] = static_cast<void(GObject::*)(const string&)>(&GObject::removeThread);

		auto agent = _state.new_usertype<Agent>("Agent", sol::base_classes, sol::bases<GObject>());
		#define _cls Agent

		addFuncSame(agent, setSprite);
		
		addFuncSame(agent, hit);
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

		auto player = _state.new_usertype<Player>("Player", sol::base_classes, sol::bases<Agent>());

		auto bullet = _state.new_usertype<Bullet>("Bullet", sol::base_classes, sol::bases<GObject>());
		bullet["makeParams"] = sol::overload(
			[](SpaceVect pos, SpaceFloat angle)->shared_ptr<object_params> { return Bullet::makeParams(pos, angle); },
			[](SpaceVect pos, SpaceFloat angle, SpaceVect vel)->shared_ptr<object_params> { return Bullet::makeParams(pos, angle, vel); },
			[](SpaceVect pos, SpaceFloat angle, SpaceVect vel, SpaceFloat angularVel)->shared_ptr<object_params> { return Bullet::makeParams(pos, angle, vel,angularVel); }
		);

		auto torch = _state.new_usertype<Torch>("Torch", sol::base_classes, sol::bases <GObject>());
#define _cls Torch
		addFuncSame(torch, getActive);
		addFuncSame(torch, setActive);
		addFuncSame(torch, hit);
	}
}
