//
//  Slime.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Slime.hpp"

Slime1::Slime1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	AgentMapForward(0.875),
	Enemy(collectible_id::hm1)
{
}

const string Slime1::baseAttributes = "slime1";
const string Slime1::properName = "Slime I";

DamageInfo Slime1::touchEffect() const {
	return DamageInfo{ 5.0f, Attribute::slimeDamage, DamageType::touch };
}

Slime2::Slime2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	AgentMapForward(1.75),
	Enemy(collectible_id::hm2)
{
}

const string Slime2::baseAttributes = "slime2";
const string Slime2::properName = "Slime II";

DamageInfo Slime2::touchEffect() const {
	return DamageInfo{ 15.0f, Attribute::slimeDamage, DamageType::touch };
}
