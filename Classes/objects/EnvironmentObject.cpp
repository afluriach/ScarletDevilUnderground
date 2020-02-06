//
//  EnvironmentObject.cpp
//  Koumachika
//
//  Created by Toni on 2/5/20.
//
//

#include "Prefix.h"

#include "EnvironmentObject.hpp"
#include "value_map.hpp"

EnvironmentObject::EnvironmentObject(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<object_properties> props
) :
	GObject(MapParamsPointUp(), physics_params(GType::environment, PhysicsLayers::eyeLevelHeight, args, props->mass)),
	props(props)
{
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
