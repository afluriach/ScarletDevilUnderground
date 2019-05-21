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
#include "graphics_context.hpp"
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
	fuseSound = playSoundSpatial("sfx/bomb_fuse.wav");
}

void Bomb::update()
{
	GObject::update();
	AudioSourceObject::_update();

	timerDecrement(countdown);

	if (countdown <= 0.0) {
		detonate();
	}
}

void Bomb::detonate()
{
	if (detonated) return;
	detonated = true;

	stopSound(fuseSound);
	explosion(this, getBlastRadius(), getDamageInfo());
	
	string sfxRes = getExplosionSound();
	if(!sfxRes.empty())
		playSoundSpatial(getExplosionSound());

	space->addGraphicsAction(
		&graphics_context::setSpriteTexture,
		spriteID,
		string("sprites/explosion.png")
	);
	space->removeObjectWithAnimation(this, bombAnimationAction(getBlastRadius() / explosionSpriteRadius, false));

	LightID light = space->addLightSource(CircleLightArea{
		getPos(),
		getBlastRadius()*1.5,
		Color4F::ORANGE,
		0.5
	});
	space->addGraphicsAction(&graphics_context::autoremoveLightSource, light, 1.0f);
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
