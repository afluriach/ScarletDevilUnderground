//
//  graphics_types.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "graphics_types.h"

shared_ptr<AmbientLightArea> AmbientLightArea::create(
	SpaceVect pos,
	SpaceVect dimensions,
	Color4F color
) {
	auto result = make_shared<AmbientLightArea>();

	result->origin = pos;
	result->dimensions = dimensions;
	result->color = color;

	return result;
}


shared_ptr<CircleLightArea> CircleLightArea::create(
	SpaceVect pos,
	SpaceFloat radius,
	Color4F color,
	float flood
) {
	auto result = make_shared<CircleLightArea>();

	result->origin = pos;
	result->radius = radius;
	result->color = color;
	result->flood = flood;

	return result;
}

shared_ptr<ConeLightArea> ConeLightArea::create(
	SpaceVect pos,
	SpaceFloat radius,
	SpaceFloat width,
	Color4F color
) {
	auto result = make_shared<ConeLightArea>();

	result->origin = pos;
	result->radius = radius;
	result->angleWidth = width;
	result->color = color;

	return result;
}

shared_ptr<SpriteLightArea> SpriteLightArea::create(
	SpaceVect pos,
	const string& spritePath,
	Color4F color,
	float scale
) {
	auto result = make_shared<SpriteLightArea>();

	result->origin = pos;
	result->texName = spritePath;
	result->color = color;
	result->scale = scale;

	return result;
}

