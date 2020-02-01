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
	const ValueMap& args,
	shared_ptr<enemy_properties> props
) :
	Agent(
		space,id,
		GType::enemy,
		props->isFlying ? flyingLayers : onGroundLayers,
		args,
		props->attributes,
		props->radius,
		props->mass
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

AttributeMap Enemy::getBaseAttributes() const {
	auto result = app::getAttributes(props->attributes);
	if (result.empty()) {
		log("EnemyImpl: unknown attributes set %s", props->attributes);
	}
	return result;
}

bool Enemy::hasEssenceRadar() const {
	return props->detectEssence;
}

SpaceFloat Enemy::getRadarRadius() const {
	return props->viewRange;
}

SpaceFloat Enemy::getDefaultFovAngle() const {
	return props->viewAngle;
}

string Enemy::getSprite() const {
	return props->sprite;
}

shared_ptr<LightArea> Enemy::getLightSource() const {
	return props->lightSource;
}

string Enemy::initStateMachine(){
	return props->ai_package;
}

string Enemy::getProperName() const {
	return props->name;
}

string Enemy::getTypeName() const {
	return props->typeName;
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
