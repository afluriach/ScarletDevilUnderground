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

environment_object_properties::environment_object_properties()
{}

environment_object_properties::~environment_object_properties()
{}

bool EnvironmentObject::conditionalLoad(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<environment_object_properties> props
) {
    auto objects = space->scriptVM->_state["objects"];
	auto cls = objects[props->clsName];

    if (cls.valid()) {
		sol::function f = cls["conditionalLoad"];

		if (f && !f(space, id)) {
			log0("object load canceled");
			return false;
		}
	}

	return true;
}

EnvironmentObject::EnvironmentObject(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<environment_object_properties> props
) :
	GObject(
		space,
		id,
		object_params(args),
		physics_params(
			props->interactible ? enum_bitwise_or(GType,environment,interactible) : GType::environment,
			props->layers,
			props->mass
		),
		props
	),
	props(props)
{
	if (props->scriptName.size() > 0) {
        auto objects = space->scriptVM->_state["objects"];
		auto cls = objects[props->scriptName];
        if (cls.valid()) {
			scriptObj = cls(this);
		}
	}
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
	if (scriptObj && hasMethod("interact")) {
		return runVoidScriptMethod<Player*>("interact", p);
	}
}

bool EnvironmentObject::hit(DamageInfo damage, SpaceVect n)
{
	if (hasMethod("hit")) {
		runVoidScriptMethod<DamageInfo, SpaceVect>("hit", damage, n);
	}

	return true;
}

void EnvironmentObject::init()
{
	GObject::init();

	if (hasMethod("initialize")) {
		runVoidScriptMethod("initialize");
	}
}

void EnvironmentObject::update()
{
	GObject::update();

	if (hasMethod("update")) {
		runVoidScriptMethod("update");
	}
}

shared_ptr<sprite_properties> EnvironmentObject::getSprite() const {
	return props->sprite;
}

GraphicsLayer EnvironmentObject::sceneLayer() const {
	return GraphicsLayer::ground;
}
