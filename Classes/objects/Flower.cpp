//
//  Flower.cpp
//  Koumachika
//
//  Created by Toni on 3/9/19.
//
//

#include "Prefix.h"

#include "Flower.h"

Flower::Flower(GSpace* space, ObjectIDType id, const string& name, const string& color, const SpaceVect& pos) :
	GObject(PosAngleParams(pos, float_pi / 2.0), physics_params(0.5, -1.0)),
	color(color)
{
}

Flower::Flower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(), physics_params(0.5, -1.0)),
	color(args.at("color").asString()) {
}
