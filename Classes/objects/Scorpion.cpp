//
//  Scorpion.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Scorpion.hpp"

Scorpion1::Scorpion1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
}

const string Scorpion1::baseAttributes = "scorpion1";
const string Scorpion1::properName = "Scorpion I";

DamageInfo Scorpion1::touchEffect() const {
	return DamageInfo{ 5.0f, Attribute::poisonDamage, DamageType::touch };
}

Scorpion2::Scorpion2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
}

const string Scorpion2::baseAttributes = "scorpion2";
const string Scorpion2::properName = "Scorpion II";

DamageInfo Scorpion2::touchEffect() const {
	return DamageInfo{ 15.0f, Attribute::poisonDamage, DamageType::touch };
}
