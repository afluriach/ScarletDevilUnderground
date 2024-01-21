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
#include "xml_impl.hpp"

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
	Element element = Element::none;
	bool success = false;

	getElementAttr(elem, "element", &element);

	if (element != Element::none) {
		*result = new MagicEffectDescImpl<ApplyDamage, Element>(elem->Name(), element);
		success = true;
	}

	return success;
}

bool damageOverTime(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result)
{
	Element element = Element::none;
	bool success = false;

	getElementAttr(elem, "element", &element);

	if (element != Element::none) {
		*result = new MagicEffectDescImpl<DamageOverTime, Element>(elem->Name(), element);
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
		log1("Unknown effect type: %s", _type);
	}

	return success;
}

}
