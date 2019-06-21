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
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "Player.hpp"

Enemy::Enemy(collectible_id drop_id) :
drop_id(drop_id)
{
}

void Enemy::runDamageFlicker()
{
	if (spriteID != 0) {
		space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			spriteID,
			flickerAction(0.3f, 1.2f, 81)
		);
	}
}

bool Enemy::hit(DamageInfo damage)
{
	if (!Agent::hit(damage))
		return false;

	runDamageFlicker();
	playSoundSpatial("sfx/enemy_damage.wav");
	return true;
}

void Enemy::onRemove()
{
	Agent::onRemove();
	if(drop_id != collectible_id::nil){
		space->createObject(Collectible::create(space, drop_id, getPos()));
	}
	space->registerEnemyDefeated(typeid(*this));
}
