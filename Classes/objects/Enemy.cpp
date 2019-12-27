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
#include "GSpace.hpp"
#include "Player.hpp"

Enemy::Enemy(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	const string& baseAttributes,
	SpaceFloat radius,
	SpaceFloat mass,
	string drop_id,
	bool isFlying
) :
	Agent(
		space,id,
		GType::enemy,
		isFlying ? flyingLayers : onGroundLayers,
		args,
		baseAttributes,radius,mass
	),
	drop_id(drop_id)
{
	if (space->getFrame() == 0){
		space->registerEnemyStaticLoaded(getTypeName());
	}
	else {
		space->registerEnemySpawned(getTypeName());
	}
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
	if(!drop_id.empty()){
		space->createObject(Collectible::create(space, drop_id, getPos()));
	}
	space->registerEnemyDefeated(getTypeName());
}

EnemyImpl::EnemyImpl(
	GSpace* space, ObjectIDType id, const ValueMap& args,
	shared_ptr<enemy_properties> props
) :
	Enemy(
		space,id,args,
		props->attributes,
		props->radius,
		props->mass,
		props->collectible,
		props->isFlying
	),
	props(props)
{
	if (props->firepattern.size() > 0)
		setFirePattern(props->firepattern);
}

void EnemyImpl::init()
{
	Agent::init();

	loadEffects();
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

string EnemyImpl::getSprite() const {
	return props->sprite;
}

shared_ptr<LightArea> EnemyImpl::getLightSource() const {
	return props->lightSource;
}

string EnemyImpl::initStateMachine(){
	return props->ai_package;
}

string EnemyImpl::getProperName() const {
	return props->name;
}

string EnemyImpl::getTypeName() const {
	return props->typeName;
}

void EnemyImpl::loadEffects()
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
