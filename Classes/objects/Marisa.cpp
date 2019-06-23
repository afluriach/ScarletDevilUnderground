//
//  Marisa.cpp
//  Koumachika
//
//  Created by Toni on 12/15/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "GSpace.hpp"
#include "Marisa.hpp"
#include "MarisaSpell.hpp"

class GObject;

MarisaNPC::MarisaNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent)
{}

const string Marisa::baseAttributes = "marisa";
const string Marisa::properName = "Marisa Kirisame";

Marisa::Marisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

CollectMarisa::CollectMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	MapObjForwarding(Marisa)
{}

ForestMarisa::ForestMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjParams(),
MapObjForwarding(Agent),
MapObjForwarding(Marisa)
{}
