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

bool CollectGlyph::canInteract() {
	return !hasInteracted;
}

void CollectGlyph::interact() {
	Collect* cs = dynamic_cast<Collect*>(GScene::crntScene);

	if (cs) {
		cs->registerActivation(this);
		hasInteracted = true;
	}
}

string CollectGlyph::interactionIcon() {
	return "sprites/glyph.png";
}
