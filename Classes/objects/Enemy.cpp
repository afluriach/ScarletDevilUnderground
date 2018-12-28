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
#include "Player.hpp"

Enemy::Enemy() :
RegisterUpdate<Enemy>(this)
{
}

void Enemy::runDamageFlicker()
{
	if (sprite)
		sprite->runAction(flickerAction(0.3f, 1.2f, 81));
	//		sprite->runAction(colorFlickerAction(0.3f, 4, Color3B(255, 0, 0)));
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

AttributeMap Enemy::touchEffect()
{
	return {
		{Attribute::hp, -1.0f}
	};
}
