//
//  lua_gspace.cpp
//  Koumachika
//
//  Created by Toni on 1/26/20.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "LuaAPI.hpp"
#include "physics_context.hpp"
#include "Player.hpp"

namespace Lua{
  	
	void Inst::addGSpace()
	{
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
		addFuncSame(attr, timerDecrement);
		addFuncSame(attr, timerIncrement);
		addFuncSame(attr, setFullHP);
		addFuncSame(attr, setFullMP);
		addFuncSame(attr, setFullStamina);

		auto gspace = _state.new_usertype<GSpace>(
			"GSpace",
			sol::no_constructor,
			"physics", sol::property(&GSpace::getPhysics),
			"crntSpace", sol::property(&GSpace::getCrntSpace)
		);
#define _cls GSpace

		gspace["getObjectByName"] = static_cast<GObject * (GSpace::*)(const string&) const>(&GSpace::getObject);
		gspace["getObject"] = static_cast<GObject * (GSpace::*)(const string&) const>(&GSpace::getObject);	  
		addFuncSame(gspace, createAreaSensor);
		addFuncSame(gspace, createBullet);
		addFuncSame(gspace, getPlayer);
		addFuncSame(gspace, getPlayerAsRef);
		addFuncSame(gspace, getFrame);
		addFuncSame(gspace, getObjectCount);
		addFuncSame(gspace, isObstacle);
		gspace["removeObject"] = sol::overload(
			static_cast<void(GSpace::*)(const string&)>(&GSpace::removeObject),
			static_cast<void(GSpace::*)(gobject_ref)>(&GSpace::removeObject),
			static_cast<void(GSpace::*)(GObject*)>(&GSpace::removeObject)
		);
		addFuncSame(gspace, getRandomInt);
		gspace["getRandomBool"] = sol::overload(
			static_cast<bool(GSpace::*)()>(&GSpace::getRandomBool),
			static_cast<bool(GSpace::*)(float)>(&GSpace::getRandomBool)
		);
		gspace["getRandomFloat"] = sol::overload(
			static_cast<float(GSpace::*)()>(&GSpace::getRandomFloat),
			static_cast<float(GSpace::*)(float, float)>(&GSpace::getRandomFloat)
		);

		addFuncSame(gspace, teleportPlayerToDoor);
		gspace["teleportToDoor"] = &GSpace::teleportPlayerToDoor;
		
		addFuncSame(gspace, getPath);
		addFuncSame(gspace, getWaypoint);
		addFuncSame(gspace, getRandomWaypoint);
		addFuncSame(gspace, getArea);

		addFuncSame(gspace, registerRoomMapped);

		gspace["createDialog"] = sol::overload(
			static_cast<void(GSpace::*)(string, bool)>(&GSpace::createDialog),
			static_cast<void(GSpace::*)(string, bool, zero_arity_function)>(&GSpace::createDialog)
		);
		addFuncSame(gspace, enterWorldSelect);
		addFuncSame(gspace, loadScene);

		auto gstate = newType(GState);
#define _cls GState

		gstate["addItem"] = sol::overload(
			static_cast<void(GState::*)(string)>(&GState::addItem),
			static_cast<void(GState::*)(string, unsigned int)>(&GState::addItem)
		);
		gstate["removeItem"] = sol::overload(
			static_cast<bool(GState::*)(string)>(&GState::removeItem),
			static_cast<bool(GState::*)(string, unsigned int)>(&GState::removeItem)
		);

		addFuncSame(gstate, hasItem);
		addFuncSame(gstate, getItemCount);

		addFuncSame(gstate, setAttribute);
		addFuncSame(gstate, getAttribute);
		addFuncSame(gstate, hasAttribute);
		addFuncSame(gstate, incrementAttribute);
		addFuncSame(gstate, subtractAttribute);

		auto phys = newType(physics_context);
#define _cls physics_context

		addFuncSame(phys, floorDistanceFeeler);
		addFuncSame(phys, floorLevelWallDistanceFeeler);
	}
}
