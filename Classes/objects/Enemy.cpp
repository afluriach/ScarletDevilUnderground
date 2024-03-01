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
#include "LuaAPI.hpp"
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
	if (props->firepattern.size() > 0)
		setFirePattern(props->firepattern);
  
    sol::init_script_object<Enemy>(this, params);
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

void Enemy::onZeroHP()
{
	Agent::onZeroHP();
	applyItemDrops();
}

void Enemy::onRemove()
{
	GObject::onRemove();
    App::getCrntState()->registerEnemyDefeated(getClsName());
}

void Enemy::applyItemDrops()
{
	if(!props) return;

	for(auto entry : props->itemDrops){
		bool spawn = App::getCrntState()->accumulateItemDrop(entry.first, entry.second);
		if(spawn)
			Item::create(space, entry.first, getPos());
	}
}

DamageInfo Enemy::touchEffect() const{
	return props->touchEffect;
}
