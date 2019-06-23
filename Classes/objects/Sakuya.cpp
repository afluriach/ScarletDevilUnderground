//
//  Sakuya.cpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "Sakuya.hpp"
#include "SakuyaSpell.hpp"

const string Sakuya::baseAttributes = "sakuya";

Sakuya::Sakuya(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}
