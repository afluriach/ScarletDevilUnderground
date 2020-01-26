//
//  Reimu.cpp
//  Koumachika
//
//  Created by Toni on 2/22/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "EnemyFirePattern.hpp"
#include "Reimu.hpp"
#include "value_map.hpp"

const string ReimuEnemy::baseAttributes = "reimu";
const string ReimuEnemy::properName = "Reimu Hakurei";

ReimuEnemy::ReimuEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Enemy(
		space, id, args,
		baseAttributes,
		defaultSize,
		40.0,
		"magic2",
		false
	)
{
	firePattern = make_shared<ReimuWavePattern>(this);
}
