//
//  lua_ai.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AI.hpp"
#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"

namespace Lua{
    
	template<typename T, typename... Params>
	shared_ptr<ai::Function> create(ai::StateMachine* fsm, Params... params)
	{
		return make_shared<T>(fsm, params...);
	}

    void Inst::addAI()
    {
		auto _ai = _state.create_table();
		_state["ai"] = _ai;

		_ai["isFacingTarget"] = &ai::isFacingTarget;
		_ai["isFacingTargetsBack"] = &ai::isFacingTargetsBack;
		_ai["viewAngleToTarget"] = &ai::viewAngleToTarget;

		_ai["autoUpdateFunction"] = [](shared_ptr<ai::Function> f) -> shared_ptr<ai::Function> {
			auto result = f;
			ai::Function::autoUpdateFunction(result);
			return result;
		};

		auto _update_return = _ai.new_usertype<ai::update_return>(
			"update_return",
			sol::constructors<ai::update_return(), ai::update_return(int,shared_ptr<ai::Function>)>()
		);

		auto _event_type = _ai.new_enum<ai::event_type, true>(
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

		_ai["event_type_bitfield"] = &make_enum_bitfield<ai::event_type>;

		auto event = _ai.new_usertype<ai::Event>("Event");
		#define _cls ai::Event
		
		addFuncSame(event, isBulletHit);
		addFuncSame(event, isDetectPlayer);
		addFuncSame(event, getDetectType);
		addFuncSame(event, getEndDetectType);
		addFuncSame(event, getEventType);

		#define _cls ai::Function
		auto func = _ai.new_usertype<ai::Function>(
			"Function",
			"fsm", sol::property(&ai::Function::getFSM)
		);

		addFuncSame(func, getSpace);
		func["getObject"] = &ai::Function::getObject;
		addFuncSame(func, getAgent);
		addFuncSame(func, onEnter);
		addFuncSame(func, update);
		addFuncSame(func, reset);
		addFuncSame(func, isActive);
		addFuncSame(func, isCompleted);
		addFuncSame(func, onReturn);
		addFuncSame(func, onExit);
		addFuncSame(func, onEvent);
		addFuncSame(func, getEvents);
		addFuncSame(func, getName);

		func["makeNullShared"] = []() -> shared_ptr<ai::Function> {
			return nullptr;
		};

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
		auto sm = _ai.new_usertype<ai::StateMachine>("StateMachine");

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
		addFuncSame(sm, addAlertHandler);
		addFuncSame(sm, addAlertFunction);
		addFuncSame(sm, addOnDetectHandler);
		addFuncSame(sm, addWhileDetectHandler);

		addFuncSame(sm, getSpace);
		addFuncSame(sm, getObject);
		addFuncSame(sm, getAgent);
		addFuncSame(sm, getFrame);
		addFuncSame(sm, toString);

		auto scriptFunc = _ai.new_usertype<ai::ScriptFunction>("ScriptFunction");

		scriptFunc["create"] = &create<ai::ScriptFunction, const string&>;
		scriptFunc["targetGenerator"] = &ai::ScriptFunction::targetGenerator;


		auto evade = _ai.new_usertype<ai::Evade>(
			"Evade",
			sol::base_classes, sol::bases<ai::Function>()
		);
		evade["create"] = &create<ai::Evade, GType>;

		auto flank = _ai.new_usertype<ai::Flank>(
			"Flank",
			sol::base_classes, sol::bases<ai::Function>()
		);
		flank["create"] = &create<ai::Flank, gobject_ref, double, double>;
		flank["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::Flank, SpaceFloat, SpaceFloat>;

		auto flee = _ai.new_usertype<ai::Flee>(
			"Flee",
			sol::base_classes, sol::bases<ai::Function>()
		);
		flee["create"] = &create<ai::Flee, GObject*, SpaceFloat>;
		flee["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::Flee, SpaceFloat>;

		auto flock = _ai.new_usertype<ai::Flock>(
			"Flock",
			sol::base_classes, sol::bases<ai::Function>()
		);
		flock["create"] = &create<ai::Flock>;

		auto idle = _ai.new_usertype<ai::IdleWait>(
			"IdleWait",
			sol::base_classes, sol::bases<ai::Function>()
		);
		idle["create"] = &create<ai::IdleWait>;

		auto maintain_distance = _ai.new_usertype<ai::MaintainDistance>(
			"MaintainDistance",
			sol::base_classes, sol::bases<ai::Function>()
		);
		maintain_distance["create"] = &create<ai::MaintainDistance, gobject_ref, SpaceFloat, SpaceFloat>;
		maintain_distance["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::MaintainDistance, SpaceFloat, SpaceFloat>;

		auto scurry = _ai.new_usertype < ai::Scurry>(
			"Scurry",
			sol::base_classes, sol::bases<ai::Function>()
		);
		scurry["create"] = &create<ai::Scurry, GObject*, SpaceFloat, SpaceFloat>;
		scurry["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::Scurry, SpaceFloat, SpaceFloat>;

		auto seek = _ai.new_usertype<ai::Seek>(
			"Seek",
			sol::base_classes, sol::bases<ai::Function>()
		);
		seek["create"] = &create<ai::Seek, GObject*, bool, SpaceFloat>;
		seek["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::Seek, bool, SpaceFloat>;

		auto wander = _ai.new_usertype<ai::Wander>(
			"Wander",
			sol::base_classes, sol::bases<ai::Function>()
		);
		wander["create"] = sol::overload(
			&create<ai::Wander>,
			&create<ai::Wander, SpaceFloat, SpaceFloat, SpaceFloat, SpaceFloat>
		);

		auto fireAtTarget = _ai.new_usertype<ai::FireAtTarget>(
			"FireAtTarget",
			sol::base_classes, sol::bases<ai::Function>()
		);
		fireAtTarget["create"] = &create<ai::FireAtTarget, gobject_ref>;
		fireAtTarget["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::FireAtTarget>;

		auto fireOnStress = _ai.new_usertype<ai::FireOnStress>(
			"FireOnStress",
			sol::base_classes, sol::bases<ai::Function>()
		);
		fireOnStress["create"] = &create<ai::FireOnStress, float>;

		auto boss = _ai.new_usertype<ai::BossFightHandler>(
			"BossFightHandler",
			sol::base_classes, sol::bases<ai::Function>()
		);
		boss["create"] = &create<ai::BossFightHandler, string, string>;
	}
}
