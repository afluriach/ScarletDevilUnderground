//
//  FirePattern.cpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#include "Prefix.h"

#include "FirePattern.hpp"

const SpaceFloat FirePattern::defaultLaunchDistance = 1.0;

GSpace* FirePattern::getSpace() const {
	return agent->space;
}

bool FirePattern::fireIfPossible()
{
	return fire();
}
