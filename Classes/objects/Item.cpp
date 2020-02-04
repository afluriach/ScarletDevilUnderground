//
//  Item.cpp
//  Koumachika
//
//  Created by Toni on 2/2/20.
//
//

#include "Prefix.h"

#include "Item.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "value_map.hpp"

item_properties::item_properties() :
	dimensions(0.5, 0.0)
{}

item_attributes Item::parseAttributes(const ValueMap& args)
{
	item_attributes result;

	result.name = getStringOrDefault(args, "name", "");
	result.pos = getObjectPos(args);

	return result;
}

bool Item::conditionalLoad(GSpace* space, const item_attributes& attr, shared_ptr<item_properties> props)
{
	if (attr.name.empty()) {
		log("Un-named item!");
		return true;
	}

	return !App::crntState->isObjectRemoved(space->getCrntChamber(), attr.name);
}

Item::Item(GSpace* space, ObjectIDType id, const item_attributes& attr, shared_ptr<item_properties> props) :
	GObject(
		make_shared<object_params>(space,id,attr.name, attr.pos),
		physics_params(GType::playerPickup, PhysicsLayers::ground, 0.5, -1.0, true)
	),
	props(props)
{
	auto& cls = space->scriptVM->_state["objects"][props->scriptName];

	if (cls) {
		scriptObj = cls(this);
	}

	if (props->name.empty()) {
		log("Empty item!");
	}
}

void Item::init()
{
	GObject::init();
	updateRoomQuery();
}

void Item::onPlayerContact(Player* p)
{
	if (!props->name.empty() && props->addToInventory) {
		App::crntState->addItem(props->name);
	}

	if(hasMethod("onAcquire"))
		runVoidScriptMethod<Player*>("onAcquire", p);

	if (!props->onAcquireDialog.empty()) {
		space->createDialog("dialogs/" + props->onAcquireDialog, false);
	}

	space->removeObject(this);
}
