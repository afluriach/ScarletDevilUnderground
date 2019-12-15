//
//  types.cpp
//  Koumachika
//
//  Created by Toni on 1/20/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "app_constants.hpp"
#include "Attributes.hpp"

namespace app {
	app_params params;
}

DamageInfo::DamageInfo() :
	mag(0.0f),
	element(Attribute::end),
	type(DamageType::end)
{}

DamageInfo::DamageInfo(float mag, DamageType type) :
	mag(mag),
	element(Attribute::end),
	type(type)
{}

DamageInfo::DamageInfo(float mag, Attribute element, DamageType type) : 
	mag(mag),
	element(element),
	type(type)
{}

DamageInfo DamageInfo::operator*(float rhs)
{
	return DamageInfo{ mag * rhs, element, type };
}

float app_params::getScale() const
{
	return 1.0f * width / app::baseWidth;
}

double app_params::getFrameInterval() const
{
	return secondsPerFrame;
}

SpaceRect::SpaceRect() :
	center(SpaceVect::zero),
	dimensions(SpaceVect::zero)
{}

SpaceRect::SpaceRect(const b2AABB& bb) :
	SpaceRect(bb.GetCenter(), bb.GetDimensions())
{
}

SpaceRect::SpaceRect(SpaceVect center, SpaceVect dimensions) :
	center(center),
	dimensions(dimensions)
{}

SpaceRect::SpaceRect(SpaceFloat llX, SpaceFloat llY, SpaceFloat width, SpaceFloat height) :
	center(SpaceVect(llX + width/2.0, llY + height/2.0)),
	dimensions(SpaceVect(width, height))
{}

CCRect SpaceRect::toPixelspace() const
{
	return CCRect(getMinX(), getMinY(), dimensions.x, dimensions.y) * app::pixelsPerTile;
}

double SpaceRect::getMinX() const
{
	return center.x - dimensions.x / 2.0;
}

double SpaceRect::getMinY() const
{
	return center.y - dimensions.y / 2.0;
}

double SpaceRect::getMaxX() const
{
	return center.x + dimensions.x / 2.0;
}

double SpaceRect::getMaxY() const
{
	return center.y + dimensions.y / 2.0;
}

SpaceVect SpaceRect::getLLCorner() const
{
	return SpaceVect(getMinX(), getMinY());
}

SpaceVect SpaceRect::getURCorner() const
{
	return SpaceVect(getMaxX(), getMaxY());
}

bool SpaceRect::intersectsRect(const SpaceRect& rect) const
{
	return !(getMaxX() < rect.getMinX() ||
		rect.getMaxX() <      getMinX() ||
		getMaxY() < rect.getMinY() ||
		rect.getMaxY() <      getMinY());
}

bool SpaceRect::containsPoint(const SpaceVect& point) const
{
	return (
		point.x >= getMinX() && point.x <= getMaxX() &&
		point.y >= getMinY() && point.y <= getMaxY()
	);
}

const GType bombObstacles = enum_bitwise_or4(GType, enemy, environment, wall, bomb);
const GType bulletObstacles = enum_bitwise_or4(GType, player, enemy, environment, wall);
const GType interactibleObjects = enum_bitwise_or(GType, npc, environment);
const GType agentObstacles = enum_bitwise_or5(GType, wall, enemy, environment, npc, player);

bool isValidChamber(ChamberID id)
{
	return id >= ChamberID::begin && id < ChamberID::end;
}

const PhysicsLayers onGroundLayers = enum_bitwise_or(PhysicsLayers, floor, ground);
const PhysicsLayers eyeLevelHeightLayers = enum_bitwise_or3(PhysicsLayers, floor, ground, eyeLevel);
const PhysicsLayers flyingLayers = PhysicsLayers::ground;
