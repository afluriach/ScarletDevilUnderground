//
//  CollectGlyph.cpp
//  Koumachika
//
//  Created by Toni on 12/15/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "Collect.h"
#include "CollectGlyph.hpp"
#include "GSpace.hpp"

CollectGlyph::CollectGlyph(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{
	collectScene = space->getSceneAs<Collect>();

	if (!collectScene) {
		throw runtime_error("CollectGlyph created outside of Collect scene!");
	}
}


bool CollectGlyph::canInteract() {
	return !hasInteracted;
}

void CollectGlyph::interact() {
	space->addSceneAction(
		bind(&Collect::registerActivation, collectScene, gobject_ref(this))
	);

	hasInteracted = true;
}

string CollectGlyph::interactionIcon() {
	return "sprites/glyph.png";
}
