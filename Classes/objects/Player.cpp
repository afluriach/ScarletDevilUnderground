//
//  Player.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "AreaSensor.hpp"
#include "audio_context.hpp"
#include "Bomb.hpp"
#include "Bullet.hpp"
#include "Door.hpp"
#include "FirePattern.hpp"
#include "FloorSegment.hpp"
#include "functional.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GraphicsNodes.hpp"
#include "HUD.hpp"
#include "Inventory.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "SpellDescriptor.hpp"

const float Player::centerLookHoldThresh = 0.3f;
const float Player::interactCooldownTime = 0.1f;

const float Player::hitFlickerInterval = 0.333f;

const SpaceFloat Player::interactDistance = 1.25;
const SpaceFloat Player::grazeRadius = 0.7;

Player::Player(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<agent_properties> props
) :
	Agent(
		space,
		id,
		enum_bitwise_or(GType,player,canDamage),
		params,
		props
	)
{
	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Player created outside of PlayScene!");
	}
 
    sol::init_script_object<Player>(this, params);    
}

Player::~Player()
{
}

void Player::setCrntRoom(RoomSensor* room)
{
	GObject::setCrntRoom(room);

	if (room) {
		space->updatePlayerMapLocation(room->getID());
	}
}

void Player::equipFirePatterns()
{
	inventory->firePatterns.clear();

	for (auto entry : FirePattern::playerFirePatterns)
	{
		if (App::crntState->hasItem(entry.first) || app::params.unlockAllEquips) {
			local_shared_ptr<FirePattern> pattern = entry.second(this);

			if (pattern) {
				inventory->firePatterns.add(pattern);
			}
		}
	}

    firePattern = inventory->firePatterns.getCrnt();
    
	space->addHudAction(
		&HUD::setFirePatternIcon,
		inventory->firePatterns.getIcon()
	);
}

void Player::equipSpells()
{
	inventory->spells.clear();

	for (const SpellDesc* desc : props->spellInventory)
	{
		inventory->spells.add(desc);
	}

	space->addHudAction(
		&HUD::setSpellIcon,
		inventory->spells.getIcon()
	);
}

void Player::equipPowerAttacks()
{
	inventory->powerAttacks.clear();

	if (props->attack) {
		inventory->powerAttacks.add(props->attack);
	}
 
    powerAttack = inventory->powerAttacks.getCrnt();

	space->addHudAction(
		&HUD::setPowerAttackIcon,
		inventory->powerAttacks.getIcon()
	);
}

void Player::equipItems()
{
	equipSpells();
	equipFirePatterns();
}

SpaceVect Player::getInteractFeeler() const
{
	return SpaceVect::ray(interactDistance, getAngle());
}

void Player::init()
{
	Agent::init();

	if (playScene && !space->getSuppressAction()) {
		equipFirePatterns();
		equipSpells();
		equipPowerAttacks();
		crntBomb = app::getBomb("PlayerBomb");
	}
}

void Player::onZeroHP()
{
	gameOver();
}

void Player::onPitfall()
{
	sprite.runAction(pitfallShrinkAction());
    gameOver();
}

void Player::update()
{
	Agent::update();

#if use_sound
	space->audioContext->setSoundListenerPos(getPos(), getVel(), float_pi/2.0);
#endif
    
	if (playScene) {
		space->addHudAction(&HUD::updateHUD, *attributeSystem);
	}
}

ALuint Player::playSoundSpatial(const string& path, float volume, bool loop, float yPos)
{
#if use_sound
	ALuint soundSource = space->audioContext->playSound(
		path,
		volume,
		loop
	);

	return soundSource;
#else
    return 0;
#endif
}

AttributeMap Player::getAttributeUpgrades() const
{
	return App::crntState->attributeUpgrades;
}

void Player::onBulletHitTarget(Bullet* bullet, Agent* target)
{
	applyCombo( (*this)[Attribute::comboPerAttack] );
}

bool Player::hit(DamageInfo damage, SpaceVect n){
	if (!Agent::hit(damage, n))
		return false;

	if (!damage.damageOverTime) {
		playSoundSpatial("sfx/player_damage.wav");
	}

	return true;
}

void Player::useDoor(Door* interactTarget)
{
	string destMap = interactTarget->getDestinationMap();
	string dest = interactTarget->getDestination();

	if (destMap.empty()) {
		Door* dest = interactTarget->getAdjacent();
		if (dest){
			moveToDestinationDoor(dest);
		}
	}
	else
	{
        space->loadScene(destMap, dest);
	}
}

void Player::moveToDestinationDoor(Door* dest)
{
	setPos(dest->getEntryPosition());
	setVel(SpaceVect::zero);
	setDirection(dest->getEntryDirection());
}

void Player::applyUpgrade(Attribute attr, float val)
{
	modifyAttribute(attr, val);
	App::crntState->applyAttributeUpgrade(attr, val);
}

void Player::applyCombo(int b)
{
	modifyAttribute(Attribute::combo, b);
}

void Player::gameOver()
{
	if (!GScene::suppressGameOver) {
#if use_sound
		space->audioContext->playSound("sfx/player_death.wav", 0.5f);
#endif

		space->addSceneAction(
			[=]()->void { playScene->triggerGameOver(); }
		);
	}
}
