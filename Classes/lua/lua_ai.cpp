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
#include "Bomb.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "SpellDescriptor.hpp"

namespace Lua{
    
	template<typename T, typename... Params>
	local_shared_ptr<ai::Function> create(ai::StateMachine* fsm, Params... params)
	{
		return make_local_shared<T>(fsm, params...);
	}

    void Inst::addAI()
    {
		auto _ai = _state.create_table();
		_state["ai"] = _ai;

		_ai["isFacingTarget"] = &ai::isFacingTarget;
		_ai["isFacingTargetsBack"] = &ai::isFacingTargetsBack;
		_ai["viewAngleToTarget"] = &ai::viewAngleToTarget;

		_ai["autoUpdateFunction"] = [](local_shared_ptr<ai::Function> f) -> local_shared_ptr<ai::Function> {
			auto result = f;
			ai::Function::autoUpdateFunction(result);
			return result;
		};

		auto _update_return = _ai.new_usertype<ai::update_return>(
			"update_return",
			sol::constructors<
				ai::update_return(),
				ai::update_return(int,float,local_shared_ptr<ai::Function>)
			>(),
			"idx", sol::property(&ai::update_return::get_idx),
			"update", sol::property(&ai::update_return::get_update),
			"f", sol::property(&ai::update_return::get_f)
		);

		_update_return["makePush"] = &ai::update_return::makePush;
		_update_return["makePop"] = &ai::update_return::makePop;
		_update_return["makeSteady"] = &ai::update_return::makeSteady;

		_update_return["isPop"] = &ai::update_return::isPop;
		_update_return["isSteady"] = &ai::update_return::isSteady;

		#define _cls ai::Function
		auto func = _ai.new_usertype<ai::Function>(
			"Function",
			"agent", sol::property(&ai::Function::getAgent),
			"fsm", sol::property(&ai::Function::getFSM),
			"object", sol::property(&ai::Function::getObject),
			"space", sol::property(&ai::Function::getSpace)
		);

		addFuncSame(func, onEnter);
		addFuncSame(func, update);
		addFuncSame(func, reset);
		addFuncSame(func, isActive);
		addFuncSame(func, isCompleted);
		addFuncSame(func, onReturn);
		addFuncSame(func, onExit);

		addFuncSame(func, bulletBlock);
		addFuncSame(func, bulletHit);
		addFuncSame(func, detectEnemy);
		addFuncSame(func, endDetectEnemy);
		addFuncSame(func, detectBullet);
		addFuncSame(func, detectBomb);
		addFuncSame(func, enemyRoomAlert);
		addFuncSame(func, zeroHP);
		addFuncSame(func, zeroStamina);

		addFuncSame(func, getName);

		addFuncSame(func, castSpell);
		addFuncSame(func, castSpellManual);
		addFuncSame(func, isSpellActive);
		addFuncSame(func, stopSpell);

		addFuncSame(func, aimAtTarget);
		addFuncSame(func, fire);

		func["makeNullShared"] = []() -> local_shared_ptr<ai::Function> {
			return nullptr;
		};

		#define _cls ai::Thread
		auto thread = _state.new_usertype<ai::Thread>(
			"Thread",
			sol::constructors < ai::Thread( ai::StateMachine*) > ()
		);
		_ai["Thread"] = thread;

		addFuncSame(thread, update);
		addFuncSame(thread, push);
		addFuncSame(thread, pop);
		addFuncSame(thread, popToRoot);
		addFuncSame(thread, getTop);
		addFuncSame(thread, getStack);
		addFuncSame(thread, getMainFuncName);

		#define _cls ai::StateMachine
		auto sm = _ai.new_usertype<ai::StateMachine>(
			"StateMachine",
			"agent", sol::property(&ai::StateMachine::getAgent),
			"object", sol::property(&ai::StateMachine::getObject),
			"space", sol::property(&ai::StateMachine::getSpace)
		);

		sm["pushFunction"] = static_cast<void(ai::StateMachine::*)(local_shared_ptr<ai::Function>)>(&ai::StateMachine::pushFunction);

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
		wander["makeTargetFunctionGenerator"] = &ai::targetFunctionGeneratorAdapter<ai::Wander>;

		auto follow_path = _ai.new_usertype<ai::FollowPath>(
			"FollowPath",
			sol::base_classes, sol::bases<ai::Function>()
		);
		follow_path["create"] = &create<ai::FollowPath, Path, bool, bool>;

		auto lookTowardsFire = _ai.new_usertype<ai::LookTowardsFire>(
			"LookTowardsFire",
			sol::base_classes, sol::bases<ai::Function>()
		);
		lookTowardsFire["create"] = &create<ai::LookTowardsFire, bool>;

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

		auto throw_bomb = _ai.new_usertype<ai::ThrowBombs>(
			"ThrowBombs",
			sol::base_classes, sol::bases<ai::Function>()
		);
		throw_bomb["create"] = &create<ai::ThrowBombs, gobject_ref, local_shared_ptr<bomb_properties>, SpaceFloat, SpaceFloat>;
		throw_bomb["makeTargetFunctionGenerator"] = &ai::makeTargetFunctionGenerator<ai::ThrowBombs, local_shared_ptr<bomb_properties>, SpaceFloat, SpaceFloat>;
	}
}
