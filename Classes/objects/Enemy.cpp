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

EnemyImpl::EnemyImpl(
	GSpace* space, ObjectIDType id, const ValueMap& args,
	shared_ptr<enemy_properties> props
) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(props->collectible),
	props(props)
{

}

DamageInfo EnemyImpl::touchEffect() const{
	return props->touchEffect;
}

AttributeMap EnemyImpl::getBaseAttributes() const {
	auto result = app::getAttributes(props->attributes);
	if (result.empty()) {
		log("EnemyImpl: unknown attributes set %s", props->attributes);
	}
	return result;
}

bool EnemyImpl::hasEssenceRadar() const {
	return props->detectEssence;
}

SpaceFloat EnemyImpl::getRadarRadius() const {
	return props->viewRange;
}

SpaceFloat EnemyImpl::getDefaultFovAngle() const {
	return props->viewAngle;
}

SpaceFloat EnemyImpl::getMass() const {
	return props->mass;
}

PhysicsLayers EnemyImpl::getLayers() const {

	return props->isFlying ? PhysicsLayers::ground : enum_bitwise_or(PhysicsLayers, ground, floor);
}

string EnemyImpl::getSprite() const {
	return props->sprite;
}

shared_ptr<LightArea> EnemyImpl::getLightSource() const {
	return props->lightSource;
}

string EnemyImpl::initStateMachine(){
	return props->ai_package;
}
