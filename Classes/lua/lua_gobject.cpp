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
#include "GObject.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"

namespace Lua{
    
    void Inst::addGObject()
    {
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
		addFuncSame(gobject, setInhibitSpellcasting);

		addFuncSame(gobject, stopSpell);
		addFuncSame(gobject, printFSM);
			
		gobject["getAsAgent"] = &GObject::getAs<Agent>;
		gobject["removeThreadByName"] = static_cast<void(GObject::*)(const string&)>(&GObject::removeThread);

		auto agent = _state.new_usertype<Agent>("Agent", sol::base_classes, sol::bases<GObject>());
		#define _cls Agent

		addFuncSame(agent, getAttribute);
		addFuncSame(agent, getAttributeSystem);
		addFuncSame(agent, getBulletAttributes);

		addFuncSame(agent, isFiringSuppressed);
		addFuncSame(agent, setFiringSuppressed);
		addFuncSame(agent, isMovementSuppressed);
		addFuncSame(agent, setMovementSuppressed);

		auto player = _state.new_usertype<Player>("Player", sol::base_classes, sol::bases<Agent>());

		auto bullet = _state.new_usertype<Bullet>("Bullet", sol::base_classes, sol::bases<GObject>());
		bullet["makeParams"] = &Bullet::makeParams;
	}
}
