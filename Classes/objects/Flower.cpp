//
//  Flower.cpp
//  Koumachika
//
//  Created by Toni on 3/9/19.
//
//

#include "Prefix.h"

#include "Flower.h"
#include "macros.h"

Flower::Flower(GSpace* space, ObjectIDType id, const string& name, const string& color, const SpaceVect& pos) :
	GObject(space, id, name, pos, float_pi / 2.0),
	color(color) {
}

Flower::Flower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	color(args.at("color").asString()) {
}
