//
//  Items.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "App.h"
#include "FirePattern.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Player.hpp"
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
	MapObjForwarding(GObject)
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

CircleLightArea Spellcard::getLightSource() const
{
	return CircleLightArea{
		getPos(),
		3.0,
		toColor4F(Color3B(94, 145, 140)),
		0.5f
	};
}

void Spellcard::initializeGraphics()
{
	ImageSprite::initializeGraphics();
	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		spriteID,
		spellcardFlickerTintAction()
	);
}

void Spellcard::onAcquire()
{
	space->getState()->itemRegistry.insert(name);
	space->removeObject(this);
	space->getObjectAs<Player>("player")->equipSpells();
	space->getObjectAs<Player>("player")->equipFirePatterns();
}
