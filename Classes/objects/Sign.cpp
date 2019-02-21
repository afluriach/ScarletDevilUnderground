//
//  Sign.cpp
//  Koumachika
//
//  Created by Toni on 2/17/19.
//
//

#include "Prefix.h"

#include "macros.h"
#include "Sign.hpp"
#include "value_map.hpp"

Sign::Sign(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{
	dialogRes = getStringOrDefault(args, "dialog", "");
	setInitialAngle(float_pi / 2.0);
}