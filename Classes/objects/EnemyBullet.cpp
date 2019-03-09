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

#define cons(x) x::x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) : \
    GObject(space, id, "", pos, angle), \
    Bullet(agent), \
    EnemyBullet(agent), \
    BulletImpl(&props) \
{} \

EnemyBullet::EnemyBullet(Agent* agent) :
	Bullet(agent)
{}

void EnemyBullet::invalidateGraze()
{
	grazeValid = false;
}

FireBullet::FireBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	MaxSpeedImpl(speed)
{}

WaterBullet::WaterBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
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

StarBullet::StarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, SpaceFloat speed, SpaceFloat radius, const string& color) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	MaxSpeedImpl(speed),
	color(color)
{}

AttributeMap StarBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

const bullet_properties IceFairyBullet::props = {
	0.1,
	6.0,
	0.3,
	0.83,
	"sprites/ice_fairy_bullet.png",
	{ { Attribute::hp, -1 }, { Attribute::iceDamage, 25 } }
};

cons(IceFairyBullet)

const bullet_properties LauncherBullet::props = {
	0.1,
	10.0,
	0.3,
	0.83,
	"sprites/launcher_bullet.png",
	hp_damage_map(1.0f)
};

LauncherBullet::LauncherBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	BulletImpl(&props)
{}

const bullet_properties Fairy1Bullet::props = {
	0.1,
	6.0,
	0.3,
	0.83,
	"sprites/ice_fairy_bullet.png",
	hp_damage_map(5.0f)
};

cons(Fairy1Bullet)

const bullet_properties GreenFairyBullet::props = {
	0.1,
	6.0,
	0.15,
	0.83,
	"sprites/green_fairy_bullet.png",
	hp_damage_map(3.0f)
};

cons(GreenFairyBullet)

const bullet_properties RumiaBullet::props = {
	0.1,
	4.5,
	0.15,
	0.83,
	"sprites/rumia_bullet.png",
	hp_damage_map(5.0f)
};

cons(RumiaBullet)

const bullet_properties RumiaPinwheelBullet::props = {
	0.1,
	4.5,
	0.1,
	0.83,
	"sprites/rumia_bullet.png",
	hp_damage_map(2.0f)
};

cons(RumiaPinwheelBullet)

const bullet_properties RumiaDemarcationBullet::props = {
	0.1,
	4.5,
	0.1,
	0.83,
	"sprites/rumia_demarcation_bullet.png",
	hp_damage_map(5.0f)
};

cons(RumiaDemarcationBullet)

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, Agent* agent, const SpaceVect& pos, SpaceFloat angular_velocity) :
GObject(space,id,"", pos, 0.0),
Bullet(agent),
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

const bullet_properties YinYangOrb::props = {
	0.1,
	4.5,
	0.5,
	0.5,
	"sprites/yin-yang-orb.png",
	hp_damage_map(10.0f)
};

YinYangOrb::YinYangOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	BulletImpl(&props)
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
