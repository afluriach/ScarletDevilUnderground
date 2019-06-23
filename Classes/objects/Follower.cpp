//
//  Follower.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "Follower.hpp"
#include "GSpace.hpp"

const string Follower::baseAttributes = "ff";

Follower::Follower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	Agent(space, id, args),
	Enemy(collectible_id::nil)
{}
