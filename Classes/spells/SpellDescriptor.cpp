//
//  SpellDescriptor.cpp
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"
#include "Spell.hpp"
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

	GSpace::scriptVM->getField<string>(obj, "name", &result.name);
	GSpace::scriptVM->getField<string>(obj, "description", &result.description);
	GSpace::scriptVM->getField<string>(obj, "icon", &result.icon);
	GSpace::scriptVM->getField<SpaceFloat>(obj, "length", &result.length);
	GSpace::scriptVM->getField<SpaceFloat>(obj, "updateInterval", &result.updateInterval);
	GSpace::scriptVM->getField<spell_cost>(obj, "cost", &result.cost);

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

Spell* ScriptedSpellDescriptor::generate(GObject* caster, unsigned int id) const
{
	return allocator_new<ScriptedSpell>(caster, this, id, clsName);
}
