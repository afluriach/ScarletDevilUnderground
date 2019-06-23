//
//  Stalker.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "EnemySpell.hpp"
#include "Stalker.hpp"
#include "TeleportPad.hpp"

const string Stalker::baseAttributes = "stalker"; 

Stalker::Stalker(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic1)
{}

void Stalker::update()
{
	Enemy::update();

	SpaceFloat dp = getVel().length() * app::params.secondsPerFrame;
	modifyAttribute(Attribute::stamina, -dp);
}

void Stalker::teleport(SpaceVect pos)
{
	attributeSystem.setFullStamina();
	
	GObject::teleport(pos);
}
