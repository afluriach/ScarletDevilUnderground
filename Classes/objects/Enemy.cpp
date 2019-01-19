//
//  Enemy.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "Enemy.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "Player.hpp"

Enemy::Enemy(collectible_id drop_id) :
drop_id(drop_id),
RegisterUpdate<Enemy>(this)
{
}

void Enemy::runDamageFlicker()
{
	if (spriteID != 0) {
		space->runSpriteAction(spriteID, flickerAction(0.3f, 1.2f, 81));
	}
}

void Enemy::onTouchPlayer(Player* player) {
	touchTarget = player;

	onDetect(player);
}

void Enemy::endTouchPlayer()
{
	touchTarget = nullptr;
}

//hit will be registered every frame, in case contact is maintained for longer than the hit protection time.
void Enemy::update()
{
	if (touchTarget.isValid() && !touchTarget.get()->isProtected()) {
		touchTarget.get()->hit(touchEffect(), nullptr);
	}
}

void Enemy::onRemove()
{
	if(drop_id != collectible_id::nil){
		space->createObject(Collectible::create(drop_id, getPos()));
	}
}


AttributeMap Enemy::touchEffect()
{
	return {
		{Attribute::hp, -1.0f}
	};
}
