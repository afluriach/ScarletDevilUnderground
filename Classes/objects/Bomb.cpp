//
//  Bomb.cpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#include "Prefix.h"

#include "Bomb.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "SpellUtil.hpp"

const SpaceFloat Bomb::explosionSpriteRadius = 2.0;

Bomb::Bomb(shared_ptr<object_params> params, shared_ptr<bomb_properties> props) :
	GObject(params, physics_params(enum_bitwise_or(GType, bomb, canDamage), onGroundLayers, 0.5, 1.0)),
	props(props)
{
}

void Bomb::init()
{
	GObject::init();
	updateRoomQuery();

	countdown = getFuseTime();
	fuseSound = playSoundSpatial("sfx/bomb_fuse.wav");
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
	space->removeObjectWithAnimation(this, bombAnimationAction(getBlastRadius() / explosionSpriteRadius, false).generator);

	LightID light = space->addLightSource(
		CircleLightArea::create(getBlastRadius()*1.5,Color4F::ORANGE,0.5),
		getPos(),
		0.0
	);
	space->addGraphicsAction(&graphics_context::autoremoveLightSource, light, 1.0f);
}

bool Bomb::hit(DamageInfo damage, SpaceVect n)
{
	if (damage.isExplosion()) {
		detonate();
	}

	return damage.isExplosion();
}
