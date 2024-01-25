//
//  object_properties.cpp
//  Koumachika
//
//  Created by Toni on 1/23/24.
//

//#include "object_properties.hpp"

object_properties::object_properties()
{}

object_properties::~object_properties()
{}

bomb_properties::bomb_properties()
{}

bomb_properties::~bomb_properties()
{}

bullet_properties::bullet_properties()
{}

bullet_properties::~bullet_properties()
{}

bullet_properties bullet_properties::clone() {
	return *this;
}

effectarea_properties::effectarea_properties()
{}

effectarea_properties::~effectarea_properties()
{}

environment_object_properties::environment_object_properties()
{}

environment_object_properties::~environment_object_properties()
{}

floorsegment_properties::floorsegment_properties()
{}

floorsegment_properties::~floorsegment_properties()
{}

item_properties::item_properties()
{}

item_properties::~item_properties()
{}

dialog_entry::dialog_entry() {}

dialog_entry::dialog_entry(string dialog) :
	dialog(dialog)
{}

dialog_entry::dialog_entry(
	function<bool(NPC*)> condition,
	function<void(NPC*)> effect,
	string dialog,
	bool once
) :
	condition(condition),
	effect(effect),
	dialog(dialog),
	once(once)
{}

npc_properties::~npc_properties()
{
}