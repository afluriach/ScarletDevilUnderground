//
//  Sign.cpp
//  Koumachika
//
//  Created by Toni on 2/17/19.
//
//

#include "Prefix.h"

#include "Sign.hpp"
#include "value_map.hpp"

Sign::Sign(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), MapRectPhys(-1.0))
{
	dialogRes = getStringOrDefault(args, "dialog", "");
}
