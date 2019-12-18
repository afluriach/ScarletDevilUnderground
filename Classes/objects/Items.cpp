//
//  Items.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "FirePattern.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Player.hpp"
#include "Spell.hpp"
#include "value_map.hpp"

ForestBook1::ForestBook1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(InventoryObject)
{}

void ForestBook1::onAcquire(Player* player)
{
	space->createDialog("dialogs/book_acquired", false);
}

GraveyardBook1::GraveyardBook1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(InventoryObject)
{}

void GraveyardBook1::onAcquire(Player* player)
{
	space->createDialog("dialogs/book_acquired", false);
}

bool Spellcard::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	string spellName = getStringOrDefault(args, "spell", "");

	if (spellName.empty()) {
		log("Spellcard without spell name.");
		return false;
	}

	return !space->getState()->hasItem(spellName);
}

Spellcard::Spellcard(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(InventoryObject)
{
	name = getStringOrDefault(args, "spell", "");

	if (name.empty()) {
		log("Spellcard, name not provided.");
	}
	else if (!Spell::getDescriptorByName(name) &&
		FirePattern::playerFirePatterns.find(name)  == FirePattern::playerFirePatterns.end()) {
		log("Descriptor not found for %s!", name.c_str());
	}
}

shared_ptr<LightArea> Spellcard::getLightSource() const
{
	return app::getLight("spellcard");
}

void Spellcard::initializeGraphics()
{
	GObject::initializeGraphics();
	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		spriteID,
		spellcardFlickerTintAction().generator
	);
}

void Spellcard::onAcquire(Player* player)
{
	space->getState()->itemRegistry.insert(name);
	space->removeObject(this);
	player->equipSpells();
	player->equipFirePatterns();
}
