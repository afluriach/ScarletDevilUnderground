//
//  CollectGlyph.cpp
//  Koumachika
//
//  Created by Toni on 12/15/18.
//
//

#include "Prefix.h"

#include "Collect.h"
#include "CollectGlyph.hpp"
#include "GSpace.hpp"
#include "object_params.hpp"
#include "value_map.hpp"

CollectGlyph::CollectGlyph(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), physics_params( enum_bitwise_or(GType, environment,interactible), PhysicsLayers::ground, args, -1.0))
{
	collectScene = space->getSceneAs<Collect>();

	if (!collectScene) {
		throw runtime_error("CollectGlyph created outside of Collect scene!");
	}
}

bool CollectGlyph::canInteract(Player* p) {
	return !hasInteracted;
}

void CollectGlyph::interact(Player* p) {
	space->addSceneAction(
		bind(&Collect::registerActivation, collectScene, gobject_ref(this))
	);

	hasInteracted = true;
}

string CollectGlyph::interactionIcon(Player* p) {
	return "sprites/glyph.png";
}
