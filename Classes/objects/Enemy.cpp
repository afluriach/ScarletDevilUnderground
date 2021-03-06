//
//  Enemy.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "audio_context.hpp"
#include "Enemy.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "Item.hpp"
#include "MagicEffect.hpp"
#include "Player.hpp"

Enemy::Enemy(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<enemy_properties> props
) :
	Agent(
		space,
		id,
		enum_bitwise_or(GType, enemy,canDamage),
		params,
		props
	),
	props(props)
{
	if (space->getFrame() == 0){
		space->registerEnemyStaticLoaded(getClsName());
	}
	else {
		space->registerEnemySpawned(getClsName());
	}

	if (props->firepattern.size() > 0)
		setFirePattern(props->firepattern);
}

Enemy::~Enemy()
{
}

void Enemy::runDamageFlicker()
{
	if (spriteID != 0) {
		space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			spriteID,
			flickerAction(0.3f, 1.2f, 81).generator
		);
	}
}

bool Enemy::hit(DamageInfo damage, SpaceVect n)
{
	if (!Agent::hit(damage, n))
		return false;

	if (!damage.damageOverTime) {
		runDamageFlicker();
		playSoundSpatial("sfx/enemy_damage.wav");
	}

	return true;
}

void Enemy::onRemove()
{
	Agent::onRemove();
	if(!props->collectible.empty()){
		space->createObject( Item::create(space, props->collectible, getPos()) );
	}
	space->registerEnemyDefeated(getClsName());
}

void Enemy::init()
{
	Agent::init();

	loadEffects();
}

DamageInfo Enemy::touchEffect() const{
	return props->touchEffect;
}

void Enemy::loadEffects()
{
	vector<string> effectNames = splitString(props->effects, ",");

	for (string name : effectNames) {
		auto effectDesc = app::getEffect(name);

		if (effectDesc) {
			applyMagicEffect(effectDesc, effect_attributes(0.0f, -1.0f));
		}
		else {
			log("Unknown MagicEffect: %s", name);
		}
	}
}
