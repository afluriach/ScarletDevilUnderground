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

item_properties::item_properties()
{}

item_attributes Item::parseAttributes(const ValueMap& args)
{
	item_attributes result;

	result.name = getStringOrDefault(args, "name", "");
	result.pos = getObjectPos(args);

	return result;
}

bool Item::conditionalLoad(GSpace* space, const item_attributes& attr, local_shared_ptr<item_properties> props)
{
	if (attr.name.empty()) {
		log("Un-named item!");
		return true;
	}

	return !App::crntState->isObjectRemoved(space->getCrntChamber(), attr.name);
}

ObjectGeneratorType Item::create(GSpace* space, string items, SpaceVect pos)
{
	vector<string> tokens;
	string actual;
	if (items.empty())
		return GObject::null_object_factory();

	tokens = splitString(items, ",");

	if (tokens.size() > 1) {
		int idx = space->getRandomInt(0, tokens.size() - 1);
		actual = tokens[idx];
	}
	else {
		actual = items;
	}

	auto props = app::getItem(actual);
	item_attributes attr{ pos };

	return GObject::make_object_factory<Item>(attr, props);
}

Item::Item(GSpace* space, ObjectIDType id, const item_attributes& attr, local_shared_ptr<item_properties> props) :
	GObject(
		space,
		id,
		object_params(attr.pos),
		physics_params(GType::item, PhysicsLayers::ground, 0.5, -1.0, true)
	),
	props(props)
{
	auto& cls = space->scriptVM->_state["objects"][props->scriptName];

	if (cls) {
		scriptObj = cls(this);
	}
}

Item::~Item()
{
}

void Item::init()
{
	GObject::init();
	updateRoomQuery();
}

void Item::onPlayerContact(Player* p)
{
	bool canAcquire = true;
	if (hasMethod("canAcquire")) {
		canAcquire = runScriptMethod<bool, Player*>("canAcquire", p);
	}

	if (!canAcquire)
		return;

	if (!props->clsName.empty() && props->addToInventory) {
		App::crntState->addItem(props->clsName);
	}

	if(hasMethod("onAcquire"))
		runVoidScriptMethod<Player*>("onAcquire", p);

	if (!props->onAcquireDialog.empty()) {
		space->createDialog("dialogs/" + props->onAcquireDialog, false);
	}

	space->removeObject(this);
}
