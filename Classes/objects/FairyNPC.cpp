//
//  FairyNPC.cpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "FairyNPC.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "value_map.hpp"

const string FairyMaid::baseAttributes = "fairyMaid";

FairyMaid::FairyMaid(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(
		space,id,args,
		baseAttributes,
		defaultSize,
		10.0
	)
{
	dialog = getStringOrDefault(args, "dialog", "");
}

bool BlueFairyNPC::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int level = getIntOrDefault(args, "level", 0);
	return level > space->getState()->getAttribute("BlueFairyLevel");
}

BlueFairyNPC::BlueFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(
		space, id, args,
		"",
		defaultSize,
		-1.0
	)
{
	level = getIntOrDefault(args, "level", 0);
}

string BlueFairyNPC::getDialog()
{
	if (level > space->getState()->getAttribute("BlueFairyLevel") + 1) return "dialogs/blue_fairy_no";
	else {
		if (space->getState()->getAttribute("mushroomCount") >= level) return "dialogs/blue_fairy_satisfied";
		else return "dialogs/blue_fairy_request_"+boost::lexical_cast<string>(level);
	}
}

void BlueFairyNPC::onDialogEnd()
{
	if (level == space->getState()->getAttribute("BlueFairyLevel") + 1 && space->getState()->getAttribute("mushroomCount") >= level) {
		space->getState()->incrementAttribute("BlueFairyLevel");
		space->getState()->subtractAttribute("mushroomCount", level);

		space->removeObject(this);

		space->addHudAction<string,int>(&HUD::setObjectiveCounter, "sprites/mushroom.png", space->getState()->getAttribute("mushroomCount"));
	} 
}

bool GhostFairyNPC::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int level = getIntOrDefault(args, "level", 0);
	int levelCount = 4;

	if (level > 0) {
		return space->getState()->isChamberCompleted("Graveyard" + level);
	}
	else {
		return true;
	}
}

GhostFairyNPC::GhostFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(
		space, id, args,
		"",
		defaultSize,
		10.0
	)
{
}
