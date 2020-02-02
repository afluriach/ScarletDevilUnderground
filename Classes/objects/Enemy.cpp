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
#include "Player.hpp"

Enemy::Enemy(
	GSpace* space,
	ObjectIDType id,
	const agent_attributes& attr,
	shared_ptr<enemy_properties> props
) :
	Agent(
		space,
		id,
		enum_bitwise_or(GType, enemy,canDamage),
		attr,
		props
	),
	props(props)
{
	if (space->getFrame() == 0){
		space->registerEnemyStaticLoaded(getTypeName());
	}
	else {
		space->registerEnemySpawned(getTypeName());
	}

	if (props->firepattern.size() > 0)
		setFirePattern(props->firepattern);
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

	runDamageFlicker();
	playSoundSpatial("sfx/enemy_damage.wav");
	return true;
}

void Enemy::onRemove()
{
	Agent::onRemove();
	if(!props->collectible.empty()){
		space->createObject(Collectible::create(space, props->collectible, getPos()));
	}
	space->registerEnemyDefeated(getTypeName());
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
			applyMagicEffect(effectDesc, 0.0f, -1.0f);
		}
		else {
			log("Unknown MagicEffect: %s", name);
		}
	}
}
