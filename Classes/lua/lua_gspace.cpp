//
//  lua_gspace.cpp
//  Koumachika
//
//  Created by Toni on 1/26/20.
//
//

#include "Prefix.h"

#include "Graphics.h"
#include "LuaAPI.hpp"
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
		addFuncSame(attr, hasHitProtection);
		addFuncSame(attr, setTimedProtection);
		addFuncSame(attr, timerDecrement);
		addFuncSame(attr, timerIncrement);
		addFuncSame(attr, setFullHP);
		addFuncSame(attr, setFullMP);
		addFuncSame(attr, setFullStamina);

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

		auto gspace = newType(GSpace);
#define _cls GSpace

		gspace["createObject"] = static_cast<gobject_ref(GSpace::*)(const ValueMap&)>(&GSpace::createObject);
		gspace["getObjectByName"] = static_cast<GObject * (GSpace::*)(const string&) const>(&GSpace::getObject);
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
		gspace["getRandomFloat"] = sol::overload(
			static_cast<float(GSpace::*)()>(&GSpace::getRandomFloat),
			static_cast<float(GSpace::*)(float, float)>(&GSpace::getRandomFloat)
		);

		addFuncSame(gspace, registerRoomMapped);

		gspace["createDialog"] = sol::overload(
			static_cast<void(GSpace::*)(string, bool)>(&GSpace::createDialog),
			static_cast<void(GSpace::*)(string, bool, zero_arity_function)>(&GSpace::createDialog)
		);
		addFuncSame(gspace, enterWorldSelect);
		addFuncSame(gspace, triggerSceneCompleted);

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

		addFuncSame(gstate, _registerChamberCompleted);
		addFuncSame(gstate, registerChamberAvailable);
		addFuncSame(gstate, isChamberAvailable);
		addFuncSame(gstate, isChamberCompleted);

		addFuncSame(gstate, setAttribute);
		addFuncSame(gstate, getAttribute);
		addFuncSame(gstate, hasAttribute);
		addFuncSame(gstate, incrementAttribute);
		addFuncSame(gstate, subtractAttribute);
	}
}
