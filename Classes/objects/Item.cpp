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

bool Item::conditionalLoad(GSpace* space, const object_params& params, local_shared_ptr<item_properties> props)
{
	if (params.name.empty()) {
		log("Un-named item!");
		return true;
	}

	return !space->getAreaStats().isObjectRemoved(params.name);
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
	object_params attr(pos);

	return GObject::make_object_factory<Item>(attr, props);
}

Item::Item(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<item_properties> props
) :
	GObject(
		space,
		id,
		object_params(params.pos),
		physics_params(GType::item, PhysicsLayers::ground, -1.0, true),
		props
	),
	props(props)
{
    auto objects = space->scriptVM->_state["objects"];
	auto cls = objects[props->scriptName];

    if (cls.valid()) {
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
