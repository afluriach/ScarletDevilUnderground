//
//  lua_magic.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AttributeEffects.hpp"
#include "MiscMagicEffects.hpp"
#include "MagicEffect.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "spell_types.hpp"

template<typename T, typename... Params>
local_shared_ptr<MagicEffect> createEffect(Params... params)
{
	return make_local_shared<T>(params...);
}

namespace Lua{
    
	void Inst::addMagic()
	{
		auto effects = _state.create_table();
		_state["effects"] = effects;

		auto spells = _state.create_table();
		_state["spells"] = spells;

		auto effattr = _state.new_usertype<effect_attributes>(
			"effect_attributes",
			sol::constructors<
				effect_attributes(float, float),
				effect_attributes(float,float,float,DamageType)
			>(),
			rw_prop(effect_attributes, magnitude),
			rw_prop(effect_attributes, length),
			rw_prop(effect_attributes, radius),
			rw_prop(effect_attributes, type)
		);

		auto effdesc = _state.new_usertype<MagicEffectDescriptor>(
			"MagicEffectDescriptor",
			"flags", sol::property(&MagicEffectDescriptor::getFlags),
			"typeName", sol::property(&MagicEffectDescriptor::getTypeName)
		);
		effdesc["canApply"] = &MagicEffectDescriptor::canApply;

#define _cls MagicEffect
		auto effect = _state.new_usertype<MagicEffect>(
			"MagicEffect"
			"MagicEffect",
			"target", sol::property(&MagicEffect::getTarget),
			"agent", sol::property(&MagicEffect::getAgent),
			"length", sol::property(&MagicEffect::getLength),
			"magnitude", sol::property(&MagicEffect::getMagnitude),
			"state", sol::property(&MagicEffect::getState),
			"flags", sol::property(&MagicEffect::getFlags)
		);

		auto flags = _state.new_enum<effect_flags, true>(
			"effect_flags",
			{
				enum_entry(effect_flags, none),

				enum_entry(effect_flags, immediate),
				enum_entry(effect_flags, durable),

				enum_entry(effect_flags, active),
				enum_entry(effect_flags, agent),
			}
		);

		addFuncSame(effect, getSpace);
		addFuncSame(effect, isImmediate);
		addFuncSame(effect, isDurable);
		addFuncSame(effect, isActive);
		addFuncSame(effect, isAgentEffect);
		addFuncSame(effect, remove);

		effect["onEnter"] = &MagicEffect::init;
		effect["update"] = &MagicEffect::update;
		effect["onExit"] = &MagicEffect::end;

		auto spellcost = _state.new_usertype<spell_cost>(
			"spell_cost",
			rw_prop(spell_cost,initial_mp),
			rw_prop(spell_cost, initial_stamina),
			rw_prop(spell_cost, ongoing_mp),
			rw_prop(spell_cost, ongoing_stamina)
		);
#define _cls spell_cost
		addFuncSame(spellcost, initial_mp);
		addFuncSame(spellcost, initial_stamina);
		addFuncSame(spellcost, ongoing_mp);
		addFuncSame(spellcost, ongoing_stamina);

		addFuncSame(spellcost, none);
		addFuncSame(spellcost, initialMP);
		addFuncSame(spellcost, initialStamina);
		addFuncSame(spellcost, ongoingMP);

		auto spellparams = _state.new_usertype<spell_params>(
			"spell_params",
			sol::constructors<
				spell_params(),
				spell_params(SpaceFloat),
				spell_params(SpaceFloat, SpaceFloat), 
				spell_params(SpaceFloat,SpaceFloat,spell_cost)
			>()
		);
#define _cls spell_params

		auto spell = _state.new_usertype<Spell>("Spell");
#define _cls Spell

		spell["stop"] = &Spell::stop;

		spell["getID"] = &Spell::getID;
		spell["getName"] = &Spell::getName;
		spell["getCost"] = &Spell::getCost;
		spell["getDescriptor"] = &Spell::getDescriptor;
		spell["getCasterObject"] = &Spell::getCasterAs<GObject>;
		spell["getCasterAsAgent"] = &Spell::getCasterAs<Agent>;
		spell["getCasterAsPlayer"] = &Spell::getCasterAs<Player>;
		spell["getSpace"] = &Spell::getSpace;

		spell["getBulletAttributes"] = &Spell::getBulletAttributes;
		spell["spawnBullet"] = &Spell::spawnBullet;

		spell["launchBullet"] = sol::overload(
			[](Spell* spell, local_shared_ptr<bullet_properties> props, SpaceVect displacement, SpaceFloat angle) -> gobject_ref {
				return spell->launchBullet(props, displacement, angle);
			},
			[](Spell* spell, local_shared_ptr<bullet_properties> props, SpaceVect displacement, SpaceFloat angle, SpaceFloat angularVelocity ) -> gobject_ref {
				return spell->launchBullet(props, displacement, angle, angularVelocity);
			},
			[](Spell* spell,local_shared_ptr<bullet_properties> props,SpaceVect displacement,SpaceFloat angle,SpaceFloat angularVelocity,bool obstacleCheck) -> gobject_ref {
				return spell->launchBullet(props, displacement, angle, angularVelocity, obstacleCheck);
			}
		);

		auto spell_desc = _state.new_usertype<SpellDesc>("SpellDescriptor");
#define _cls SpellDesc

		addFuncSame(spell_desc, getName);
		addFuncSame(spell_desc, getDescription);
		addFuncSame(spell_desc, getIcon);

		addFuncSame(spell_desc, getCost);
	}

}
