//
//  SakuyaNPC.cpp
//  Koumachika
//
//  Created by Toni on 12/27/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "SakuyaNPC.hpp"

const string SakuyaNPC::baseAttributes = "sakuyaNPC";

SakuyaNPC::SakuyaNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(space,id,args,baseAttributes,defaultSize, 40.0)
{}
