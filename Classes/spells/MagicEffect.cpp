//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "GObject.hpp"
#include "MagicEffect.hpp"

MagicEffect::MagicEffect(GObject* agent, float magnitude) :
agent(agent),
magnitude(magnitude),
crntState(state::created)
{}

GSpace* MagicEffect::getSpace() const {
	return agent->space;
}
