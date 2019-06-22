//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "MagicEffect.hpp"

MagicEffect::MagicEffect(gobject_ref target, float magnitude) :
target(target),
magnitude(magnitude),
crntState(state::created)
{}

GSpace* MagicEffect::getSpace() const {
	return target.get()->space;
}
