//
//  Items.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Spell.hpp"
#include "value_map.hpp"

ForestBook1::ForestBook1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{}

void ForestBook1::onAcquire()
{
	space->createDialog("dialogs/book_acquired", false);
}

GraveyardBook1::GraveyardBook1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{}

void GraveyardBook1::onAcquire()
{
	space->createDialog("dialogs/book_acquired", false);
}

Spellcard::Spellcard(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{
	name = getStringOrDefault(args, "spell", "");

	if (name.empty()) {
		log("Spellcard, name not provided.");
	}
	else if (!Spell::getDescriptorByName(name)) {
		log("Descriptor not found for %s!", name.c_str());
	}
}

void Spellcard::initializeGraphics()
{
	ImageSprite::initializeGraphics();
	space->runSpriteAction(spriteID, spellcardFlickerTintAction());
}

void Spellcard::onAcquire()
{

}
