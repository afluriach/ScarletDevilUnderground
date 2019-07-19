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
	Enemy(
		space, id, args,
		baseAttributes,
		defaultSize,
		40.0,
		collectible_id::magic1
	)
{}
