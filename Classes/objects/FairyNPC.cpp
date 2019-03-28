//
//  FairyNPC.cpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "App.h"
#include "FairyNPC.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
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

void FairyMaid::flee_player(ai::StateMachine& sm, const ValueMap& args) {
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::Flee>(target, 1.5), 1);
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);
	sm.addThread(make_shared<ai::IdleWait>(), 0);
}

void FairyMaid::idle(ai::StateMachine& sm, const ValueMap& args)
{
	sm.addThread(make_shared<ai::IdleWait>());
}

void FairyMaid::wander(ai::StateMachine& sm, const ValueMap& args)
{
	sm.addThread(make_shared<ai::Wander>());
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

void GhostFairyNPC::initStateMachine(ai::StateMachine& sm)
{
	sm.addThread(make_shared<ai::Wander>(), 1);

	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			if(!sm.isThreadRunning("Seek"))
				sm.addThread(make_shared<ai::Seek>(target, true, 1.5), 2);
		}
	);
}
