//
//  lua_types.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"
#include "MagicEffect.hpp"

namespace Lua{
    
	void Inst::addTypes()
	{
		auto gtype = _state.new_enum<GType, true>(
			"GType",
			{
				enum_entry(GType, none),
				enum_entry(GType, player),
				enum_entry(GType, playerBullet),
				enum_entry(GType, enemy),
				enum_entry(GType, enemyBullet),
				enum_entry(GType, environment),
				enum_entry(GType, wall),
				enum_entry(GType, areaSensor),
				enum_entry(GType, enemySensor),
				enum_entry(GType, playerGrazeRadar),
				enum_entry(GType, playerPickup),
				enum_entry(GType, npc),
				enum_entry(GType, floorSegment),
				enum_entry(GType, bomb),
				enum_entry(GType, all),
			}
		);

		auto layers = _state.new_enum<PhysicsLayers, true>(
			"PhysicsLayers",
			{
				enum_entry(PhysicsLayers, none),
				enum_entry(PhysicsLayers, belowFloor),
				enum_entry(PhysicsLayers, floor),
				enum_entry(PhysicsLayers, ground),
				enum_entry(PhysicsLayers, eyeLevel),

				enum_entry(PhysicsLayers, all),
			}
		);

		auto damageType = _state.new_enum <DamageType, true>(
			"DamageType",
			{
				enum_entry(DamageType, bullet),
				enum_entry(DamageType, bomb),
				enum_entry(DamageType, effectArea),
				enum_entry(DamageType, touch),
				enum_entry(DamageType, melee),
				enum_entry(DamageType, pitfall),
			}
		);

		auto _attr = _state.new_enum <Attribute, true>(
			"Attribute",
			{
				enum_entry(Attribute, hp),
				enum_entry(Attribute, mp),
				enum_entry(Attribute, stamina),
				enum_entry(Attribute, hitProtection),
				enum_entry(Attribute, spellCooldown),

				enum_entry(Attribute, maxHP),
				enum_entry(Attribute, maxMP),
				enum_entry(Attribute, maxStamina),
				enum_entry(Attribute, hitProtectionInterval),
				enum_entry(Attribute, spellCooldownInterval),

				enum_entry(Attribute, hpRegen),
				enum_entry(Attribute, mpRegen),
				enum_entry(Attribute, staminaRegen),

				enum_entry(Attribute, hpInv),
				enum_entry(Attribute, mpInv),
				enum_entry(Attribute, staminaInv),

				enum_entry(Attribute, hpRatio),
				enum_entry(Attribute, mpRatio),
				enum_entry(Attribute, staminaRatio),

				enum_entry(Attribute, keys),
				enum_entry(Attribute, combo),

				enum_entry(Attribute, attack),
				enum_entry(Attribute, attackSpeed),
				enum_entry(Attribute, bulletSpeed),

				enum_entry(Attribute, shieldLevel),

				enum_entry(Attribute, stress),
				enum_entry(Attribute, stressDecay),
				enum_entry(Attribute, stressFromHits),
				enum_entry(Attribute, stressFromBlocks),
				enum_entry(Attribute, stressFromDetects),

				enum_entry(Attribute, agility),

				enum_entry(Attribute, currentSpeed),
				enum_entry(Attribute, speedRatio),
				enum_entry(Attribute, maxSpeed),
				enum_entry(Attribute, maxAcceleration),

				enum_entry(Attribute, bombSensitivity),
				enum_entry(Attribute, bulletSensitivity),
				enum_entry(Attribute, meleeSensitivity),

				enum_entry(Attribute, iceSensitivity),
				enum_entry(Attribute, sunSensitivity),
				enum_entry(Attribute, darknessSensitivity),
				enum_entry(Attribute, poisonSensitivity),
				enum_entry(Attribute, slimeSensitivity),

				enum_entry(Attribute, iceDamage),
				enum_entry(Attribute, sunDamage),
				enum_entry(Attribute, darknessDamage),
				enum_entry(Attribute, poisonDamage),
				enum_entry(Attribute, slimeDamage),
			}
		);

		auto damageInfo = _state.new_usertype<DamageInfo>(
			"DamageInfo",
			sol::constructors<
				DamageInfo(),
				DamageInfo(float, DamageType),
				DamageInfo(float, DamageType, Attribute, float)
			>()
		);

		damageInfo["knockback"] = &DamageInfo::knockback;
		damageInfo["mag"] = &DamageInfo::mag;
		damageInfo["element"] = &DamageInfo::element;
		damageInfo["type"] = &DamageInfo::type;
		damageInfo["scale"] = &DamageInfo::operator*;

		auto vect = _state.new_usertype<SpaceVect>(
			"SpaceVect",
			sol::constructors<SpaceVect()>()
		);

		vect["ray"] = &SpaceVect::ray;

#define _cls app_params
		auto params = _state.new_usertype<app_params>("app_params");
		
		addFuncSame(params, getFrameInterval);

		auto _app = _state.create_table();
		_state["app"] = _app;

		_app["getEffect"] = &app::getEffect;

		_app["log"] = &log_print<>;
	}
}
