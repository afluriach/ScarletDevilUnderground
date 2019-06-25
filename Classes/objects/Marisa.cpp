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
	NPC(
		space, id, args,
		"",
		defaultSize,
		-1.0
	)
{}

