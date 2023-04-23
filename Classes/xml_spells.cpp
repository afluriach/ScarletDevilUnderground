//
//  xml_spells.cpp
//  Koumachika
//
//  Created by Toni on 2/16/20.
//
//

#include "Prefix.h"

#include "SpellDescriptor.hpp"
#include "SpellImpl.hpp"
#include "xml_impl.hpp"

namespace app {

bool getSpellCost(tinyxml2::XMLElement* elem, spell_cost* result)
{
	string costStr;
	getStringAttr(elem, "cost", &costStr);

	if (costStr.size() == 0)
		return false;

	vector<string> tokens = splitString(costStr, ":");
	if (tokens.size() != 2) {
		log("getSpellCost: %s, invalid cost attribute: %s", elem->Name(), costStr);
		return false;
	}
	float cost = boost::lexical_cast<float>(tokens[1]);

	if (tokens[0] == "ongoing_mp") {
		result->ongoing_mp = cost;
	}
	else if (tokens[0] == "ongoing_stamina") {
		result->ongoing_stamina = cost;
	}
	else if (tokens[0] == "initial_stamina") {
		result->initial_stamina = cost;
	}
	else if (tokens[0] == "initial_mp") {
		result->initial_mp = cost;
	}
	else {
		log("getSpellCost: %s, unknown cost type: %s", elem->Name(), tokens[0]);
		return false;
	}

	return true;
}

bool loadSpellParams(tinyxml2::XMLElement* elem, spell_params* result)
{
	spell_params params;

	params.name = elem->Name();
	getStringAttr(elem, "description", &params.description);
	getStringAttr(elem, "icon", &params.icon);

	getNumericAttr(elem, "length", &params.length);
	getNumericAttr(elem, "updateInterval", &params.updateInterval);

    getSpellCost(elem, &params.cost);

	*result = params;
	return true;
}

bool applySelfEffect(tinyxml2::XMLElement* elem, SpellDesc** result)
{
	spell_params params;
	string effectName;
	const MagicEffectDescriptor* effect = nullptr;

	loadSpellParams(elem, &params);

	getStringAttr(elem, "effect", &effectName);
	effect = app::getEffect(effectName);

	if (effect) {
		*result = new SpellDescImpl<ApplySelfEffect, const MagicEffectDescriptor*>(params, effect);
		return true;
	}

	return false;
}

bool meleeAttack(tinyxml2::XMLElement* elem, SpellDesc** result)
{
	spell_params params;
	melee_params melee;
	loadSpellParams(elem, &params);

	getNumericAttr(elem, "swingDist", &melee.swingDist);
	getNumericAttr(elem, "angleWidth", &melee.sideAngleWidth);
	getNumericAttr(elem, "launchDist", &melee.launchDist);
	getNumericAttr(elem, "fireCount", &melee.fireCount);

	getSubObject<bullet_properties>(elem, "melee", &melee.melee, false);
	getSubObject<bullet_properties>(elem, "bullet", &melee.bullet, false);

	if (melee.sideAngleWidth) {
		melee.sideAngleWidth = toRads(melee.sideAngleWidth);
	}

	*result = new SpellDescImpl<MeleeAttack, melee_params>(params, melee);
	return true;
}

bool circlingBullets(tinyxml2::XMLElement* elem, SpellDesc** result)
{
	spell_params params;
	circling_bullets_params circle;
	loadSpellParams(elem, &params);

	getSubObject<bullet_properties>(elem, "bullet", &circle.bullet, false);

	getNumericAttr(elem, "distance", &circle.distance);
	getNumericAttr(elem, "angularSpeed", &circle.angularSpeed);
	getNumericAttr(elem, "bulletCount", &circle.bulletCount);

	if (getNumericAttr(elem, "speed", &circle.angularSpeed)) {
		circle.angularSpeed = linearToAngularSpeed(circle.angularSpeed, circle.distance);
	}

	*result = new SpellDescImpl<CirclingBullets, circling_bullets_params>(params, circle);
	return true;

}

bool scriptedSpell(tinyxml2::XMLElement* elem, SpellDesc** result)
{
	string cls;

	getStringAttr(elem, "class", &cls);

	if (cls == "auto") {
		cls = elem->Name();
	}

	if (cls.size() > 0) {
		*result = new ScriptedSpellDescriptor(cls);
	}

	return true;
}

const unordered_map<string, spell_parser> spellParsers = {
	{"ApplySelfEffect", &applySelfEffect},
	{"CirclingBullets", &circlingBullets},
	{"MeleeAttack", &meleeAttack},
	{"ScriptedSpell", &scriptedSpell},
};

bool parseObject(tinyxml2::XMLElement* elem, SpellDesc** result)
{
	string _type;
	bool success = false;
	getStringAttr(elem, "type", &_type);

	auto it = spellParsers.find(_type);
	if (it != spellParsers.end()) {
		success = it->second(elem, result);
	}
	else {
		log("Unknown spell type: %s", _type);
	}

	return success;
}

}
