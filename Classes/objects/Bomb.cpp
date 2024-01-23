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
#include "SpellUtil.hpp"

const SpaceFloat Bomb::explosionSpriteRadius = 2.0;

Bomb::Bomb(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<bomb_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(
			enum_bitwise_or(GType, bomb, canDamage),
			PhysicsLayers::onGround,
			1.0
		),
		props
	),
	props(props)
{
}

Bomb::~Bomb()
{
}

GraphicsLayer Bomb::sceneLayer() const {
    return GraphicsLayer::ground;
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

DamageInfo Bomb::getDamageInfo() const {
    return props->damage;
}

SpaceFloat Bomb::getFuseTime() const {
    return props->fuseTime;
}

SpaceFloat Bomb::getBlastRadius() const {
    return props->blastRadius;
}

string Bomb::getExplosionSound() const {
    return "sfx/" + props->explosionSound + ".wav";
}

shared_ptr<sprite_properties> Bomb::getSprite() const {
    return props->sprite;
}

bool Bomb::hit(DamageInfo damage, SpaceVect n)
{
	if (damage.isExplosion()) {
		detonate();
	}

	return damage.isExplosion();
}
