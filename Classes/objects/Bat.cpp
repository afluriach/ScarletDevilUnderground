//
//  Bat.cpp
//  Koumachika
//
//  Created by Toni on 2/20/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Bat.hpp"
#include "GSpace.hpp"

const string Bat::baseAttributes = "bat";
const string Bat::properName = "Bat";

Bat::Bat(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}
