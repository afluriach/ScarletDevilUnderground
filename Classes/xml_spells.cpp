//
//  xml_spells.cpp
//  Koumachika
//
//  Created by Toni on 2/16/20.
//
//

#include "Prefix.h"

#include "SpellDescriptor.hpp"

namespace app {

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
