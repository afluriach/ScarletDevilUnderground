//
//  Facer.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "Facer.hpp"
#include "GSpace.hpp"

const string Facer::baseAttributes = "ff";
const string Facer::properName = "Facer";

Facer::Facer(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}
