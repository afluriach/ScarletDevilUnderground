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
#include "MiscMagicEffects.hpp"
#include "xml.hpp"

namespace app {

bool restoreAttribute(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result)
{
	Attribute attr = Attribute::end;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::end) {
		*result = make_local_shared< MagicEffectDescImpl<RestoreAttribute, Attribute>>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool fortifyAttribute(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result)
{
	Attribute attr = Attribute::end;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::end) {
		*result = make_local_shared< MagicEffectDescImpl<FortifyAttribute, Attribute>>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool teleport(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result)
{
	*result = make_local_shared< MagicEffectDescImpl<Teleport>>(elem->Name());
	return true;
}

bool drainFromMovement(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result)
{
	Attribute attr = Attribute::end;
	bool success = false;

	getAttributeAttr(elem, "attr", &attr);

	if (attr != Attribute::end) {
		*result = make_local_shared< MagicEffectDescImpl<DrainFromMovement, Attribute>>(elem->Name(), attr);
		success = true;
	}

	return success;
}

bool scriptedEffect(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result)
{
	string clsName;
	bool success = false;

	getStringAttr(elem, "class", &clsName);

	if (clsName == "auto")
		clsName = elem->Name();

	if (!clsName.empty()) {
		*result = make_local_shared<MagicEffectDescImpl<ScriptedMagicEffect,string>>(elem->Name(), clsName);
		success = true;
	}

	return success;
}

const unordered_map<string, effect_parser> effectParsers = {
	{"RestoreAttribute", &restoreAttribute},
	{"FortifyAttribute", &fortifyAttribute},
	{"Teleport", &teleport},
	{"DrainFromMovement", &drainFromMovement},
	{"ScriptedEffect", &scriptedEffect},
};

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result)
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
