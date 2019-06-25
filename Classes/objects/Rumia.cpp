//
//  Rumia.cpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"
#include "Rumia.hpp"
#include "RumiaSpells.hpp"
#include "value_map.hpp"

Rumia::Rumia(GSpace* space, ObjectIDType id, const ValueMap& args, const string& attributes) :
	Enemy(
		space, id, args,
		attributes,
		defaultSize,
		20.0,
		collectible_id::nil
	)
{
}

shared_ptr<LightArea> Rumia::getLightSource() const
{
	return app::getLight("rumiaEnemy");
}

const string Rumia1::baseAttributes = "rumia1";
const string Rumia1::properName = "Rumia I";

Rumia1::Rumia1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Rumia(space,id,args,baseAttributes)
{
	firePattern = make_shared<RumiaBurstPattern>(this);
}

void Rumia1::onZeroHP()
{
	space->createDialog("dialogs/rumia2", false);
	Agent::onZeroHP();
}

const string Rumia2::baseAttributes = "rumia2";
const string Rumia2::properName = "Rumia II";

Rumia2::Rumia2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Rumia(space, id, args, baseAttributes)
{
	firePattern = make_shared<RumiaBurstPattern2>(this);
}

void Rumia2::onZeroHP()
{
	Agent::onZeroHP();
}
