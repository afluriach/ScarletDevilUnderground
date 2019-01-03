//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "App.h"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "MagicEffect.hpp"
#include "SpaceLayer.h"
#include "value_map.hpp"

WaterBullet::WaterBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed) :
	GObject(space, id, "", pos, angle, true),
	MaxSpeedImpl(speed)
{}

const vector<string> StarBullet::colors = {
	"blue",
	"green",
	"grey",
	"indigo",
	"purple",
	"red",
	"yellow"
};

AttributeMap StarBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

IceFairyBullet::IceFairyBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "IceFairyBullet", pos, angle, true)
{}

LauncherBullet::LauncherBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "LauncherBullet", pos, angle, true)
{}

AttributeMap IceFairyBullet::getAttributeEffect() const{
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 25 }
	};
}

shared_ptr<MagicEffect> IceFairyBullet::getMagicEffect(gobject_ref target) {
	return nullptr;
}

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angular_velocity) :
GObject(space,id,"IllusionDialDagger", pos, true),
RegisterUpdate<IllusionDialDagger>(this)
{
    setInitialAngularVelocity(angular_velocity);
}

SpaceFloat IllusionDialDagger::targetViewAngle()
{
    GObject* target = space->getObject("player");

    if(target)
        return ai::viewAngleToTarget(this,target);
    else
        return numeric_limits<SpaceFloat>::infinity();
}

void IllusionDialDagger::launch()
{
    GObject* target = space->getObject("player");

    if(target){
        setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
        setAngularVel(0.0f);
    }
    else
        debug_log("player missing");
}

void IllusionDialDagger::update()
{
	if (drawNode) {
		drawNode->setRotation(-toDegrees(getAngle()));
	}
}

void IllusionDialDagger::initializeGraphics(SpaceLayer* layer)
{
	ImageSprite::initializeGraphics(layer);

	SpaceVect _dim = getDimensions();
	float hWidth = to_float(_dim.x / 2.0 * App::pixelsPerTile);
	float hHeight = to_float(_dim.y / 2.0 * App::pixelsPerTile);
	drawNode = DrawNode::create();
	drawNode->drawSolidRect(Vec2(-hWidth,-hHeight), Vec2(hWidth,hHeight), Color4F(.66f, .75f, .66f, .7f));

	layer->getLayer(GraphicsLayer::agentOverlay)->addChild(drawNode);
	drawNode->setVisible(false);

}

FlandreBigOrb1::FlandreBigOrb1(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"bigOrb1", pos, angle, true)
{}

AttributeMap FlandreBigOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -3 }
	};
}

FlandreFastOrb1::FlandreFastOrb1(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"fastOrb1", pos, angle, true)
{}

AttributeMap FlandreFastOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

FlandreCounterClockBullet::FlandreCounterClockBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle, true)
{}

AttributeMap FlandreCounterClockBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

RumiaFastOrb1::RumiaFastOrb1(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "fastOrb1", pos, angle, true)
{}

AttributeMap RumiaFastOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

CirnoLargeIceBullet::CirnoLargeIceBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"bigIce1", pos, angle, true)
{}

AttributeMap CirnoLargeIceBullet::getAttributeEffect() const {
	return {
		{Attribute::hp, -1},
		{Attribute::iceDamage, 50}
	};
}

shared_ptr<MagicEffect> CirnoLargeIceBullet::getMagicEffect(gobject_ref target)
{
	return nullptr;
}

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle, true)
{}

AttributeMap CirnoIceShieldBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 50 }
	};
}
