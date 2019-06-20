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
	GObject(make_shared<object_params>(space, id, name, pos, float_pi / 2.0)),
	CircleBody(0.5),
	color(color) {
}

Flower::Flower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(make_shared<object_params>(space, id, args)),
	CircleBody(0.5),
	color(args.at("color").asString()) {
}
