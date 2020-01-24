//
//  types.cpp
//  Koumachika
//
//  Created by Toni on 1/20/19.
//
//

#include "Prefix.h"

#include "app_constants.hpp"

namespace app {
	app_params params;
}

DamageInfo DamageInfo::bomb(float mag)
{
	return DamageInfo(mag, DamageType::bomb);
}

DamageInfo DamageInfo::bullet(float mag)
{
	return DamageInfo(mag, DamageType::bullet);
}

DamageInfo DamageInfo::melee(float mag)
{
	return DamageInfo(mag, DamageType::melee);
}

DamageInfo::DamageInfo() :
	mag(0.0f),
	knockback(0.0f),
	element(Attribute::end),
	type(DamageType::end)
{}

DamageInfo::DamageInfo(float mag, DamageType type) :
	mag(mag),
	knockback(0.0f),
	element(Attribute::end),
	type(type)
{}

DamageInfo::DamageInfo(float mag, DamageType type, Attribute element, float knockback) :
	mag(mag),
	type(type),
	element(element),
	knockback(knockback)
{}

DamageInfo DamageInfo::operator*(float rhs)
{
	return DamageInfo(mag * rhs, type, element, knockback * rhs);
}

bool DamageInfo::isExplosion()
{
	return type == DamageType::bomb && mag > 0.0f;
}

bool DamageInfo::isValid()
{
	return mag > 0.0f && type != DamageType::end;
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
const GType agentObstacles = enum_bitwise_or5(GType, wall, enemy, environment, npc, player);

GType getBaseType(GType type)
{
	return bitwise_and(GType, type, GType::all);
}

const PhysicsLayers onGroundLayers = enum_bitwise_or(PhysicsLayers, floor, ground);
const PhysicsLayers eyeLevelHeightLayers = enum_bitwise_or3(PhysicsLayers, floor, ground, eyeLevel);
const PhysicsLayers flyingLayers = PhysicsLayers::ground;
