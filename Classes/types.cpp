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
#include "enum.h"
#include "types.h"
#include "util.h"

namespace app {
	app_params params;
}

float app_params::getScale() const
{
	return 1.0f * width / app::baseWidth;
}

SpaceRect::SpaceRect() :
	center(SpaceVect::zero),
	dimensions(SpaceVect::zero)
{}

SpaceRect::SpaceRect(const cpBB& bb) :
	SpaceRect(bb.l, bb.b, bb.r - bb.l, bb.t - bb.b)
{}

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

bool isValidChamber(ChamberID id)
{
	return id >= ChamberID::begin && id < ChamberID::end;
}
