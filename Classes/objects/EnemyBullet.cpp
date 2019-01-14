//
//  EnemyBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "MagicEffect.hpp"
#include "SpaceLayer.h"

EnemyBullet::EnemyBullet()
{}

void EnemyBullet::invalidateGraze()
{
	grazeValid = false;
}

FireBullet::FireBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed) :
	GObject(space, id, "fireBullet", pos, angle, true),
	MaxSpeedImpl(speed)
{}

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

StarBullet::StarBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed, SpaceFloat radius, const string& color) :
	color(color),
	MaxSpeedImpl(speed),
	GObject(space, id, "starBullet", pos, angle, true)
{}

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

AttributeMap LauncherBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

AttributeMap IceFairyBullet::getAttributeEffect() const{
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 25 }
	};
}

Fairy1Bullet::Fairy1Bullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle, true)
{}

AttributeMap Fairy1Bullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
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
