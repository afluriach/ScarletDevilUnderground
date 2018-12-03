//
//  Follower.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "Follower.hpp"
#include "macros.h"

const AttributeMap Follower::baseAttributes = boost::assign::map_list_of
(Attribute::speed, 1.0f)
(Attribute::acceleration, 4.5f)
;

void Follower::hit(int damage, shared_ptr<MagicEffect> effect)
{
    rotate(-0.5f*float_pi);
}
