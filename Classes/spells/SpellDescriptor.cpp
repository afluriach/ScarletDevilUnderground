//
//  SpellDescriptor.cpp
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "SpellDescriptor.hpp"

sol::table ScriptedSpellDescriptor::getClsObject(const string& clsName)
{
	sol::object result = GSpace::scriptVM->_state["spells"][clsName];
	return result;
}

ScriptedSpellDescriptor::ScriptedSpellDescriptor(string clsName) :
	clsName(clsName)
{
	//this doesn't work because the spell descriptors are constructed in a map initializer,
	//before the script VM has loaded.

	//clsObject = GSpace::scriptVM->_state["spells"][clsName];

	//if (!clsObject) {
	//	log("ScriptedSpellDescriptor created for non-existant %s!", clsName);
	//}
}

string ScriptedSpellDescriptor::getName() const
{
	auto clsObject = getClsObject(clsName);
	if (clsObject) {
		sol::object obj = clsObject["name"];

		return obj ? obj.as<string>() : string("");
	}
}

string ScriptedSpellDescriptor::getDescription() const
{
	auto clsObject = getClsObject(clsName);
	if (clsObject) {
		sol::object obj = clsObject["description"];

		return obj ? obj.as<string>() : string("");
	}
	else {
		return string();
	}
}

string ScriptedSpellDescriptor::getIcon() const
{
	auto clsObject = getClsObject(clsName);
	if (clsObject) {
		sol::object obj = clsObject["icon"];

		return obj ? obj.as<string>() : string("");
	}
	else {
		return string();
	}
}

spell_cost ScriptedSpellDescriptor::getCost() const
{
	spell_params params = ScriptedSpell::getParams(clsName);
	return params.cost;
}

shared_ptr<Spell> ScriptedSpellDescriptor::generate(GObject* caster)
{
	return make_shared<ScriptedSpell>(caster, clsName);
}

SpellGeneratorType ScriptedSpellDescriptor::getGenerator()
{
	string _name = clsName;
	return [_name](GObject* caster) -> shared_ptr<Spell> {
		return make_shared<ScriptedSpell>(caster, _name);
	};
}
