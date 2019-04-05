//
//  Bomb.cpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "Bomb.hpp"
#include "GScene.hpp"
#include "GSpace.hpp"
#include "SpellUtil.hpp"

const SpaceFloat Bomb::explosionSpriteRadius = 2.0;

Bomb::Bomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	RegisterInit<Bomb>(this)
{
	if (!vel.isZero()) {
		setInitialVelocity(vel);
	}
	setInitialAngle(float_pi * 0.5);
}

void Bomb::init()
{
	countdown = getFuseTime();
	App::playSoundSpatial("sfx/bomb_fuse.wav", toVec3(getPos()), toVec3(SpaceVect::zero));
}

void Bomb::update()
{
	GObject::update();

	timerDecrement(countdown);

	if (countdown <= 0.0) {
		detonate();
	}
}

void Bomb::detonate()
{
	explosion(this, getBlastRadius(), getDamageInfo());
	
	space->setSpriteTexture(spriteID, "sprites/explosion.png");
	space->removeObjectWithAnimation(this, bombAnimationAction(getBlastRadius() / explosionSpriteRadius, false));

	LightID light = space->addLightSource(CircleLightArea{ getPos(), getBlastRadius()*1.5, Color4F::ORANGE, 0.5 });
	space->autoremoveLightSource(light, 1.0f);
}

PlayerBomb::PlayerBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	Bomb(space, id, pos, vel)
{
}

RedFairyBomb::RedFairyBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	Bomb(space, id, pos, vel)
{
}
