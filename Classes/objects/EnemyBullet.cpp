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

EnemyBullet::EnemyBullet(Agent* agent) :
	Bullet(agent)
{}

void EnemyBullet::invalidateGraze()
{
	grazeValid = false;
}

FireBullet::FireBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent),
	MaxSpeedImpl(speed)
{}

WaterBullet::WaterBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent),
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

StarBullet::StarBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed, SpaceFloat radius, const string& color) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent),
	MaxSpeedImpl(speed),
	color(color)
{}

AttributeMap StarBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

IceFairyBullet::IceFairyBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{}

LauncherBullet::LauncherBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(nullptr)
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

Fairy1Bullet::Fairy1Bullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{}

AttributeMap Fairy1Bullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -5.0f }
	};
}

GreenFairyBullet::GreenFairyBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{}

AttributeMap GreenFairyBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -3.0f }
	};
}

shared_ptr<MagicEffect> IceFairyBullet::getMagicEffect(gobject_ref target) {
	return nullptr;
}

RumiaBullet::RumiaBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{}

AttributeMap RumiaBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -5.0f }
	};
}

RumiaPinwheelBullet::RumiaPinwheelBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{}

AttributeMap RumiaPinwheelBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -2.0f }
	};
}

RumiaDemarcationBullet::RumiaDemarcationBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{}

AttributeMap RumiaDemarcationBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -5.0f }
	};
}

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, Agent* agent, const SpaceVect& pos, SpaceFloat angular_velocity) :
GObject(space,id,"", pos, 0.0),
EnemyBullet(agent),
RectangleBody(SpaceVect(0.8, 0.175)),
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
	if (drawNodeID != 0) {
		space->setSpriteAngle(drawNodeID, -toDegrees(getAngle()));
	}
}

void IllusionDialDagger::initializeGraphics()
{
	ImageSprite::initializeGraphics();

	SpaceVect _dim = getDimensions();
	float hWidth = to_float(_dim.x / 2.0 * App::pixelsPerTile);
	float hHeight = to_float(_dim.y / 2.0 * App::pixelsPerTile);
	
	drawNodeID = space->createDrawNode(GraphicsLayer::agentOverlay, getInitialCenterPix(), 1.0f);
	space->drawSolidRect(drawNodeID, Vec2(-hWidth, -hHeight), Vec2(hWidth, hHeight), Color4F(.66f, .75f, .66f, .7f));
	space->setSpriteVisible(drawNodeID, false);
}

YinYangOrb::YinYangOrb(GSpace* space, ObjectIDType id, Agent* agent, const SpaceVect& pos, SpaceFloat angle) :
	GObject(space, id, "", pos, angle),
	EnemyBullet(agent)
{
	ricochetCount = -1;
	hitCount = -1;
	knockback = 150.0;

	setInitialAngularVelocity(float_pi);
}

SpriteLightArea YinYangOrb::getLightSource() const
{
	return SpriteLightArea{
		getPos(),
		imageSpritePath(),
		Color4F(0.5f, 0.25f, 0.25f, 1.0f),
		3.0f
	};
}

AttributeMap YinYangOrb::getAttributeEffect() const
{
	return { { Attribute::hp, -10.0f } };
}
