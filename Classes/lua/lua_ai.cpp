//
//  lua_ai.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "LuaAPI.hpp"
#include "SpellDescriptor.hpp"

namespace Lua{
    
	template<typename T, typename... Params>
	local_shared_ptr<ai::Function> create(GObject* object, Params... params)
	{
		return make_local_shared<T>(object, params...);
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

		#define _cls ai::Function
		auto func = _ai.new_usertype<ai::Function>(
			"Function",
			"object", sol::property(&ai::Function::getObject),
			"space", sol::property(&ai::Function::getSpace)
		);

		addFuncSame(func, onEnter);
		addFuncSame(func, update);
		addFuncSame(func, isActive);
		addFuncSame(func, isCompleted);
		addFuncSame(func, onExit);

		addFuncSame(func, getName);

		func["makeNullShared"] = []() -> local_shared_ptr<ai::Function> {
			return nullptr;
		};

		auto scriptFunc = _ai.new_usertype<ai::ScriptFunction>("ScriptFunction");

		scriptFunc["create"] = &create<ai::ScriptFunction, const string&>;

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

		auto flee = _ai.new_usertype<ai::Flee>(
			"Flee",
			sol::base_classes, sol::bases<ai::Function>()
		);
		flee["create"] = &create<ai::Flee, GObject*, SpaceFloat>;

		auto maintain_distance = _ai.new_usertype<ai::MaintainDistance>(
			"MaintainDistance",
			sol::base_classes, sol::bases<ai::Function>()
		);
		maintain_distance["create"] = &create<ai::MaintainDistance, gobject_ref, SpaceFloat, SpaceFloat>;

		auto scurry = _ai.new_usertype < ai::Scurry>(
			"Scurry",
			sol::base_classes, sol::bases<ai::Function>()
		);
		scurry["create"] = &create<ai::Scurry, GObject*, SpaceFloat, SpaceFloat>;

		auto seek = _ai.new_usertype<ai::Seek>(
			"Seek",
			sol::base_classes, sol::bases<ai::Function>()
		);
		seek["create"] = &create<ai::Seek, GObject*, bool, SpaceFloat>;

		auto wander = _ai.new_usertype<ai::Wander>(
			"Wander",
			sol::base_classes, sol::bases<ai::Function>()
		);
		wander["create"] = sol::overload(
			&create<ai::Wander>,
			&create<ai::Wander, SpaceFloat, SpaceFloat, SpaceFloat, SpaceFloat>
		);

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
  
        auto player_control = _ai.new_usertype<ai::PlayerControl>(
            "PlayerControl",
            sol::base_classes, sol::bases<ai::Function>()
        );
        player_control["create"] = &create<ai::PlayerControl>;
	}
}

