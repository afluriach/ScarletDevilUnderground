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

bool EnvironmentObject::conditionalLoad(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<environment_object_properties> props
) {
	auto& cls = space->scriptVM->_state["objects"][props->clsName];

	if (cls) {
		sol::function f = cls["conditionalLoad"];

		if (f && !f(space, id, args, props)) {
			log("object load canceled");
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
			args,
			props->mass
		),
		props
	),
	props(props)
{
	if (props->scriptName.size() > 0) {
		auto& cls = space->scriptVM->_state["objects"][props->scriptName];
		if (cls) {
			scriptObj = cls(this, args);
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
