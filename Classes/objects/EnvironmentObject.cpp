//
//  EnvironmentObject.cpp
//  Koumachika
//
//  Created by Toni on 2/5/20.
//
//

#include "Prefix.h"

#include "EnvironmentObject.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "value_map.hpp"

EnvironmentObject::EnvironmentObject(
	GSpace* space,
	ObjectIDType id,
	local_shared_ptr<environment_object_properties> props,
    const object_params& params
) :
	GObject(
		space,
		id,
        params,
		physics_params(
			props->interactible ? enum_bitwise_or(GType,environment,interactible) : GType::environment,
			props->layers,
			props->mass
		),
		props
	),
	props(props)
{
    init_script_object()
}

EnvironmentObject::~EnvironmentObject()
{
}

string EnvironmentObject::interactionIcon(Player* p)
{
	return props->interactionIcon;
}

bool EnvironmentObject::canInteract(Player* p)
{
	if (!props->interactible) return false;

	if (scriptObj && hasMethod("canInteract")) {
		return runScriptMethod<bool, Player*>("canInteract", p);
	}
	else {
		return true;
	}
}

void EnvironmentObject::interact(Player* p)
{
    runMethodIfAvailable("interact", p);
}

bool EnvironmentObject::hit(DamageInfo damage, SpaceVect n)
{
    runMethodIfAvailable("hit", damage, n);

	return true;
}

void EnvironmentObject::init()
{
	GObject::init();

    runMethodIfAvailable("initialize");
}

void EnvironmentObject::update()
{
	GObject::update();

    runMethodIfAvailable("update");
}

shared_ptr<sprite_properties> EnvironmentObject::getSprite() const {
	return props->sprite;
}

GraphicsLayer EnvironmentObject::sceneLayer() const {
	return GraphicsLayer::ground;
}
