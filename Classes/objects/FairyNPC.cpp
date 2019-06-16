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
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "value_map.hpp"

const AttributeMap FairyMaid::baseAttributes = {
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

const AIPackage<FairyMaid>::AIPackageMap FairyMaid::aiPackages = {
	{ "flee_player", &FairyMaid::flee_player },
	{ "idle", &FairyMaid::idle },
	{ "wander", &FairyMaid::wander },
};

FairyMaid::FairyMaid(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	DialogImpl(args),
	AIPackage<FairyMaid>(this, args, "idle")
{
}

void FairyMaid::flee_player(const ValueMap& args) {
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Flee>(&fsm, target, 1.5), 1);
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Flee");
		}
	);
	fsm.addThread(make_shared<ai::IdleWait>(&fsm), 0);
}

void FairyMaid::idle(const ValueMap& args)
{
	fsm.addThread(make_shared<ai::IdleWait>(&fsm));
}

void FairyMaid::wander(const ValueMap& args)
{
	fsm.addThread(make_shared<ai::Wander>(&fsm));
}

bool BlueFairyNPC::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int level = getIntOrDefault(args, "level", 0);
	return level > space->getState()->getBlueFairyLevel();
}

BlueFairyNPC::BlueFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{
	level = getIntOrDefault(args, "level", 0);
}

string BlueFairyNPC::getDialog()
{
	if (level > space->getState()->getBlueFairyLevel() + 1) return "dialogs/blue_fairy_no";
	else {
		if (space->getState()->mushroomCount >= level) return "dialogs/blue_fairy_satisfied";
		else return "dialogs/blue_fairy_request_"+boost::lexical_cast<string>(level);
	}
}

void BlueFairyNPC::onDialogEnd()
{
	if (level == space->getState()->getBlueFairyLevel() + 1 && space->getState()->mushroomCount >= level) {
		++space->getState()->blueFairies;
		space->getState()->mushroomCount -= level;
		space->removeObject(this);

		space->addHudAction<string,int>(&HUD::setObjectiveCounter, "sprites/mushroom.png", space->getState()->mushroomCount);
	} 
}

bool GhostFairyNPC::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int level = getIntOrDefault(args, "level", -1);
	int levelCount = enum_count(ChamberID, graveyard4, graveyard1);

	if (level == -1) {
		log("GhostFairyNPC: unknown level!");
		return false;
	}
	else if (level > levelCount) {
		log("GhostFairyNPC: invalid level %d!", level);
		return false;
	}
	else if (level == 0) {
		return true;
	}
	else {
		return space->getState()->isChamberCompleted(enum_add(ChamberID, graveyard1, level - 1));
	}
}

GhostFairyNPC::GhostFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{
}

void GhostFairyNPC::initStateMachine()
{
	fsm.addThread(make_shared<ai::Wander>(&fsm), 1);

	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if(!fsm.isThreadRunning("Seek"))
				fsm.addThread(make_shared<ai::Seek>(&fsm, target, true, 1.5), 2);
		}
	);
}
