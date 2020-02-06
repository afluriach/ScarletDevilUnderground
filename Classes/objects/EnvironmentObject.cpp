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
	const ValueMap& args,
	local_shared_ptr<environment_object_properties> props
) :
	GObject(
		MapParamsPointUp(),
		physics_params(
			props->interactible ? enum_bitwise_or(GType,environment,interactible) : GType::environment,
			props->layers,
			args,
			props->mass
		)
	),
	props(props)
{
	if (props->scriptName.size() > 0) {
		auto& cls = space->scriptVM->_state["objects"][props->scriptName];
		if (cls) {
			scriptObj = cls(this);
		}
	}
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
	return false;
}

string EnvironmentObject::getSprite() const {
	return props->sprite;
}

GraphicsLayer EnvironmentObject::sceneLayer() const {
	return GraphicsLayer::ground;
}

inline SpaceFloat EnvironmentObject::uk() const {
	return props->friction;
}
