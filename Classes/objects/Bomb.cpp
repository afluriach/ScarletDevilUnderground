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
#include "GSpace.hpp"
#include "SpellUtil.hpp"

Bomb::Bomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	RegisterInit<Bomb>(this)
{
	if (!vel.isZero()) {
		setInitialVelocity(vel);
	}
}

void Bomb::init()
{
	countdown = getFuseTime();
	App::playSoundSpatial("sfx/bomb_fuse.wav", toFmod(getPos()), toFmod(SpaceVect::zero));
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
	explosion(this, getBlastRadius(), getAttributeEffect());
	space->removeObjectWithAnimation(this, bombAnimationAction(getBlastRadius() / getRadius(), false));
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
