//
//  Facer.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "Facer.hpp"
#include "macros.h"

const AttributeMap Facer::baseAttributes = boost::assign::map_list_of
	(Attribute::speed, 1.0f)
	(Attribute::acceleration, 4.5f)
;

void Facer::hit(int damage, shared_ptr<MagicEffect> effect)
{
    rotate(float_pi/2);
}
