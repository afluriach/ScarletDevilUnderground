//
//  xml_effects.cpp
//  Koumachika
//
//  Created by Toni on 12/27/19.
//
//

#include "Prefix.h"

#include "AttributeEffects.hpp"
#include "MagicEffect.hpp"

namespace app {

bool restoreAttribute(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Attribute attr = Attribute::none;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::none) {
		*result = new MagicEffectDescImpl<RestoreAttribute, Attribute>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool fortifyAttribute(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Attribute attr = Attribute::none;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::none) {
		*result = new MagicEffectDescImpl<FortifyAttribute, Attribute>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool drainFromMovement(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Attribute attr = Attribute::none;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::none) {
		*result = new MagicEffectDescImpl<DrainFromMovement, Attribute>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool setBoolAttribute(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Attribute attr = Attribute::none;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::none) {
		*result = new MagicEffectDescImpl<SetBoolAttribute, Attribute>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool applyDamage(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Attribute element = Attribute::none;
	bool success = false;

	getAttributeAttr(elem, "element", &element);

	if (element != Attribute::none) {
		*result = new MagicEffectDescImpl<ApplyDamage, Attribute>(elem->Name(), element);
		success = true;
	}

	return success;
}

bool damageOverTime(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Attribute element = Attribute::none;
	bool success = false;

	getAttributeAttr(elem, "element", &element);

	if (element != Attribute::none) {
		*result = new MagicEffectDescImpl<DamageOverTime, Attribute>(elem->Name(), element);
		success = true;
	}

	return success;
}

bool scriptedEffect(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	string clsName;
	bool success = false;

	getStringAttr(elem, "class", &clsName);

	if (clsName == "auto")
		clsName = elem->Name();

	if (!clsName.empty()) {
		*result = new MagicEffectDescImpl<ScriptedMagicEffect,string>(elem->Name(), clsName);
		success = true;
	}

	return success;
}

const unordered_map<string, effect_parser> effectParsers = {
	{"RestoreAttribute", &restoreAttribute},
	{"FortifyAttribute", &fortifyAttribute},
	{"DrainFromMovement", &drainFromMovement},
	{"SetBoolAttribute", &setBoolAttribute},
	{"ApplyDamage", &applyDamage},
	{"DamageOverTime", &damageOverTime},
	{"ScriptedEffect", &scriptedEffect},
};

bool parseObject(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	string _type;
	bool success = false;
	getStringAttr(elem, "type", &_type);

	auto it = effectParsers.find(_type);
	if (it != effectParsers.end()) {
		success = it->second(elem, result);
	}
	else {
		log("Unknown effect type: %s", _type);
	}

	return success;
}

}
