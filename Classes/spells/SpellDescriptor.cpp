//
//  SpellDescriptor.cpp
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"
#include "SpellDescriptor.hpp"
#include "SpellImpl.hpp"

sol::table ScriptedSpellDescriptor::getClsObject(const string& clsName)
{
	sol::object result = GSpace::scriptVM->_state["spells"][clsName];
	return result;
}

spell_params ScriptedSpellDescriptor::getParams(const string& clsName)
{
	sol::table obj = getClsObject(clsName);
	spell_params result;

	sol::getField<string>(obj, "name", &result.name);
	sol::getField<string>(obj, "description", &result.description);
	sol::getField<string>(obj, "icon", &result.icon);
	sol::getField<SpaceFloat>(obj, "length", &result.length);
	sol::getField<SpaceFloat>(obj, "updateInterval", &result.updateInterval);
	sol::getField<spell_cost>(obj, "cost", &result.cost);

	return result;
}

ScriptedSpellDescriptor::ScriptedSpellDescriptor(string clsName) :
	SpellDesc(getParams(clsName)),
	clsName(clsName)
{
	//this doesn't work because the spell descriptors are constructed in a map initializer,
	//before the script VM has loaded.

	//clsObject = GSpace::scriptVM->_state["spells"][clsName];

	//if (!clsObject) {
	//	log("ScriptedSpellDescriptor created for non-existant %s!", clsName);
	//}
}

local_shared_ptr<Spell> ScriptedSpellDescriptor::generate(GObject* caster) const
{
	return make_local_shared<ScriptedSpell>(caster, this, clsName);
}
