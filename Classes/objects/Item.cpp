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

Item* Item::create(GSpace* space, string items, SpaceVect pos)
{
	vector<string> tokens;
	string actual;
	if (items.empty())
		return nullptr;

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

	return space->createObject<Item>(attr, props);
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
		physics_params(
			GType::item,
			PhysicsLayers::ground,
			-1.0,
			true,
			true
		),
		props
	),
	props(props)
{
    sol::init_script_object<Item>(this, params);
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

    runMethodIfAvailable("onAcquire", p);

	if (!props->onAcquireDialog.empty()) {
		space->createDialog("dialogs/" + props->onAcquireDialog, false);
	}

	space->removeObject(this);
}
