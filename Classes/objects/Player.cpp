//
//  Player.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "App.h"
#include "Bomb.hpp"
#include "Collectibles.hpp"
#include "controls.h"
#include "Door.hpp"
#include "EnemyBullet.hpp"
#include "FloorSegment.hpp"
#include "functional.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "Player.hpp"
#include "PlayerFirePattern.hpp"
#include "PlayerSpell.hpp"
#include "PlayScene.hpp"
#include "SpellDescriptor.hpp"
#include "Upgrade.hpp"

const float Player::interactCooldownTime = 0.1f;
const float Player::bombCooldownTime = 1.0f;

const float Player::hitFlickerInterval = 0.333f;

const SpaceFloat Player::sprintSpeedRatio = 1.5;
const SpaceFloat Player::focusSpeedRatio = 0.5;

const SpaceFloat Player::interactDistance = 1.25;
const SpaceFloat Player::grazeRadius = 0.7;

const float Player::bombCost = 1.0f;
const float Player::powerAttackCost = 25.0f;
const float Player::spellCost = 5.0f;

Player::Player(GSpace* space, ObjectIDType id, const SpaceVect& pos, Direction d) :
	Agent(space, id, "player", pos,d)
{

}

Player::Player(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(Agent)
{
	multiInit.insertWithOrder(wrap_method(Player, init, this), to_int(GObject::initOrder::postLoadAttributes));

	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Player created outside of PlayScene!");
	}
}


void Player::onPitfall()
{
	if (isRespawnActive || respawnMaskTimer > 0.0) {
		return;
	}

	space->runSpriteAction(spriteID, pitfallShrinkAction());
	startRespawn();
}

void Player::equipSpells()
{
	spells.clear();

	for (string spellName : Spell::playerSpells)
	{
		shared_ptr<SpellDesc> desc = Spell::getDescriptorByName(spellName);
		if (desc) {
			spells.push_back(desc);
		}
	}

	spellIdx = spells.size() > 0 ? 0 : -1;
}

SpaceFloat Player::getSpellLength()
{
	PlayerSpell* ps = dynamic_cast<PlayerSpell*>(crntSpell.get());

	if (!isSpellActive() || !ps) {
		return 0.0;
	}
	else {
		return ps->getLength();
	}
}

SpaceVect Player::getInteractFeeler() const
{
	return SpaceVect::ray(interactDistance, getAngle());
}

void Player::init()
{
	if (playScene && !space->getSuppressAction()) {
		space->addHudAction(&HUD::setMaxHP, to_int(attributeSystem[Attribute::maxHP]));
		space->addHudAction(&HUD::setHP, to_int(attributeSystem[Attribute::hp]));
		space->addHudAction(&HUD::setMaxMP, to_int(attributeSystem[Attribute::maxMP]));
		space->addHudAction(&HUD::setMP, to_int(attributeSystem[Attribute::mp]));
		space->addHudAction(&HUD::setMaxStamina, to_int(attributeSystem[Attribute::maxStamina]));
		space->addHudAction(&HUD::setStamina, to_int(attributeSystem[Attribute::stamina]));

		setFirePattern();

		if (getFirePattern()) {
			space->addHudAction(&HUD::setFirePatternIcon, getFirePattern()->iconPath());
		}

		equipSpells();

	}

	respawnPos = getPos();
	respawnAngle = getAngle();
}

void Player::checkMovementControls(const ControlInfo& cs)
{
	if (suppressMovement){
		return;
	}

	if (cs.isControlActionPressed(ControlAction::walk)) {
		space->setBulletBodiesVisible(true);
	}
	else if (cs.isControlActionReleased(ControlAction::walk)) {
		space->setBulletBodiesVisible(false);
	}

	setFocusMode(cs.isControlActionDown(ControlAction::walk));
	setSprintMode(cs.isControlActionDown(ControlAction::sprint));

    SpaceVect moveDir = cs.left_v;
	SpaceVect facing = cs.right_v;
	SpaceFloat speedRatio = getSpeedMultiplier();

    ai::applyDesiredVelocity(this, moveDir*getMaxSpeed() * speedRatio, getMaxAcceleration());
    
	if (moveDir.isZero()) {
		reset();
	}
    	
	if (facing.lengthSq() > 0.0) {
		setAngle(facing.toAngle());
	}
}

void Player::updateSpellControls(const ControlInfo& cs)
{
    if(!crntSpell.get() && spells.size() > 0 && spellIdx != -1)
    {
		attributeSystem.timerDecrement(Attribute::spellCooldown);
		
		if (cs.isControlActionPressed(ControlAction::spellPrev)) {
			--spellIdx;
			if (spellIdx < 0) spellIdx += spells.size();
			log("Spell %s equipped.", spells.at(spellIdx).get()->getName().c_str());
		}
		else if (cs.isControlActionPressed(ControlAction::spellNext)) {
			++spellIdx;
			if (spellIdx >= spells.size()) spellIdx -= spells.size();
			log("Spell %s equipped.", spells.at(spellIdx).get()->getName().c_str());
		}
		
		SpellDesc* equippedSpell = nullptr;

		if (spellIdx >= 0 && spellIdx < spells.size()) {
			equippedSpell = spells.at(spellIdx).get();
		}

		if (
			cs.isControlActionPressed(ControlAction::spell) &&
			equippedSpell &&
			!attributeSystem.isNonzero(Attribute::spellCooldown) &&
			attributeSystem[Attribute::mp] >= spellCost
		) {
			attributeSystem.modifyAttribute(Attribute::mp, -spellCost);
			cast(equippedSpell->generate(this));		
			attributeSystem.resetCombo();
			App::playSound("sfx/player_spellcard.wav", 1.0f);
		}
    }
}

void Player::onSpellStop()
{
	attributeSystem.setSpellCooldown();

	space->addHudAction(
		&HUD::runMagicFlicker,
		getAttribute(Attribute::spellCooldownInterval),
		hitFlickerInterval
	);
}

void Player::checkFireControls(const ControlInfo& cs)
{
	//Fire if arrow key is pressed
	if (!suppressFiring && !cs.right_v.isZero() && getFirePattern())
	{
		if (getFirePattern()->fireIfPossible()) {
			App::playSound("sfx/shot.wav", 1.0f);
		}
	}
	else if (
		!suppressFiring &&
		cs.isControlActionPressed(ControlAction::powerAttack) &&
		powerAttack &&
		getStamina() >= powerAttackCost)
	{
		if (powerAttack->fireIfPossible()) {
			App::playSound("sfx/shot.wav", 1.0f);
			attributeSystem.modifyAttribute(Attribute::stamina, -powerAttackCost);
		}
	}
}

void Player::checkBombControls(const ControlInfo& cs)
{
	timerDecrement(bombCooldown);

	if (!suppressFiring &&
		cs.isControlActionPressed(ControlAction::bomb) &&
		bombCooldown <= 0.0f &&
		attributeSystem[Attribute::mp] >= bombCost)
	{
		SpaceVect bombPos = getPos() + SpaceVect::ray(1.5, getAngle());
		if (canPlaceBomb(bombPos)) {
			space->createObject(GObject::make_object_factory<PlayerBomb>(bombPos, getVel()));
			attributeSystem.modifyAttribute(Attribute::mp, -bombCost);
			bombCooldown = bombCooldownTime;
		}
	}
}

void Player::checkItemInteraction(const ControlInfo& cs)
{
	timerDecrement(interactCooldown);

	InteractibleObject* interactible = space->interactibleObjectFeeler(this, getInteractFeeler());
	
	if(interactible && interactible->canInteract())
    {
        if(cs.isControlActionPressed(ControlAction::interact) && interactCooldown <= 0.0f){
            interactible->interact();
            interactCooldown = interactCooldownTime;
        }
    }

	space->addHudAction(
		&HUD::setInteractionIcon,
		interactible && interactible->canInteract() ? interactible->interactionIcon() : ""
	);
}

void Player::updateCombo()
{
	if (attributeSystem[Attribute::combo] >= AttributeSystem::maxComboPoints && !isComboActive) {
		isComboActive = true;
		attributeSystem.modifyAttribute(Attribute::attack, 0.25f);
		space->runSpriteAction(spriteID, comboFlickerTintAction());
	}
	else if (!attributeSystem.isNonzero(Attribute::combo) && isComboActive) {
		isComboActive = false;
		attributeSystem.modifyAttribute(Attribute::attack, -0.25f);
		space->stopSpriteAction(spriteID, cocos_action_tag::combo_mode_flicker);
		space->setSpriteColor(spriteID, Color3B::WHITE);
	}

	if (attributeSystem[Attribute::combo] > 0) {
		attributeSystem.modifyAttribute(Attribute::combo, -App::secondsPerFrame * 15.0f);
	}
}

void Player::onZeroHP()
{
	if (!GScene::suppressGameOver) {
		App::playSound("sfx/player_death.wav", 0.5f);

		if (!space->getIsRunningReplay()) {
			space->addSceneAction(
				[=]()->void { playScene->triggerGameOver(); }
			);
		}
	}
}

void Player::update()
{
	Agent::update();

	App::setSoundListenerPos(getPos(), getVel(), float_pi/2.0);

	if (playScene) {
		space->updatePlayerMapLocation(getPos());

		ControlInfo cs = space->getControlInfo();

		checkMovementControls(cs);
		checkItemInteraction(cs);

		if (!space->getSuppressAction()) {
			checkFireControls(cs);
			checkBombControls(cs);
			updateSpellControls(cs);

			updateCombo();

			if (!isSpellActive()) {
				setHudEffect(Attribute::hitProtection, Attribute::hitProtectionInterval);
			}
			else {
				setHudEffect(Attribute::hitProtection, getSpellLength());
			}

			setHudEffect(Attribute::spellCooldown, Attribute::spellCooldownInterval);
			setHudEffect(Attribute::combo, AttributeSystem::maxComboPoints);

			updateHudAttribute(Attribute::iceDamage);
			updateHudAttribute(Attribute::sunDamage);
			updateHudAttribute(Attribute::darknessDamage);
			updateHudAttribute(Attribute::poisonDamage);
			updateHudAttribute(Attribute::slimeDamage);

			space->addHudAction(&HUD::setHP, to_int(attributeSystem[Attribute::hp]));
			space->addHudAction(&HUD::setMP, to_int(attributeSystem[Attribute::mp]));
			space->addHudAction(&HUD::setStamina, to_int(attributeSystem[Attribute::stamina]));
			space->addHudAction(&HUD::setKeyCount, to_int(attributeSystem[Attribute::keys]));
		}

		timerDecrement(respawnTimer);
		timerDecrement(respawnMaskTimer);

		if (respawnTimer <= 0.0 && isRespawnActive) {
			applyRespawn();
		}

		if (powerAttack) {
			powerAttack->update();
		}
	}
}

SpaceFloat Player::getSpeedMultiplier()
{
	if (isSprintActive) {
		return sprintSpeedRatio;
	}
	else if (isFocusActive) {
		return focusSpeedRatio;
	}
	else {
		return 1.0;
	}
}

void Player::setFocusMode(bool b)
{
	isFocusActive = b;
	setShieldActive(b);
}

void Player::setSprintMode(bool b)
{
	isSprintActive = b;
}

unsigned int Player::getKeyCount() const
{
	return attributeSystem[Attribute::keys];
}

void Player::useKey()
{
	if (getKeyCount() > 0) {
		attributeSystem.modifyAttribute(Attribute::keys, -1.0f);
	}
}

AttributeMap Player::getAttributeUpgrades() const
{
	return space->getState()->getUpgrades();
}

void Player::applyAttributeModifier(Attribute id, float val)
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return;
	}

	attributeSystem.modifyAttribute(id, val);
}

void Player::setProtection()
{
	attributeSystem.setProtection();
}

void Player::setTimedProtection(SpaceFloat seconds)
{
	attributeSystem.setTimedProtection(seconds);
}

void Player::resetProtection()
{
	attributeSystem.resetProtection();;
}

void Player::onBulletCollide(Bullet* b)
{
	grazeContacts.erase(b);
	Agent::onBulletCollide(b);
}

void Player::onBulletHitTarget(Bullet* bullet, Agent* target)
{
	applyCombo(6);
}

bool Player::hit(AttributeMap attributeEffect, shared_ptr<MagicEffect> effect){
	if (!Agent::hit(attributeEffect, effect))
		return false;

	attributeSystem.setHitProtection();
	attributeSystem.set(Attribute::combo, 0.0f);

	space->runSpriteAction(
		spriteID,
		flickerAction(
			hitFlickerInterval,
			attributeSystem[Attribute::hitProtectionInterval],
			81
		)
	);

	space->addHudAction(
		&HUD::runHealthFlicker,
		attributeSystem[Attribute::hitProtectionInterval],
		hitFlickerInterval
	);

	App::playSound("sfx/player_damage.wav", 1.0f);
}

void Player::onCollectible(Collectible* coll)
{
	if (canApplyAttributeEffects(coll->getEffect())) {
		applyAttributeEffects(coll->getEffect());
		space->removeObject(coll);
		App::playSound("sfx/powerup.wav", 1.0f);
	}
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
		space->addSceneAction(
			[destMap, dest]()->void { App::runOverworldScene(destMap, dest); }
		);
	}
}

void Player::moveToDestinationDoor(Door* dest)
{
	setPos(dest->getEntryPosition());
	setVel(SpaceVect::zero);
	setDirection(dest->getEntryDirection());

	respawnPos = dest->getEntryPosition();
	respawnAngle = dirToPhysicsAngle(dest->getEntryDirection());
}

void Player::applyUpgrade(Upgrade* up)
{
	Attribute at = up->attribute;
	float step = AttributeSystem::upgradeAttributes.at(at).step;
	attributeSystem.modifyAttribute(at, step);

	switch (at)
	{
	case Attribute::maxHP:
		attributeSystem.modifyAttribute(Attribute::hp, step);
		space->addHudAction(&HUD::setMaxHP,to_int(attributeSystem[Attribute::maxHP]));
	break;
	case Attribute::maxMP:
		space->addHudAction(&HUD::setMaxMP, to_int(attributeSystem[Attribute::maxMP]));
	break;
	case Attribute::bulletCount:
		setFirePattern();
	break;
	}

	space->getState()->registerUpgrade(at, up->upgrade_id);
	space->removeObject(up);
}

void Player::onGrazeTouch(Bullet* bullet)
{
	grazeContacts.insert(bullet);
}

//Effect is applied after the graze "radar" loses contact.
void Player::onGrazeCleared(Bullet* bullet)
{
	//If the bullet was removed from graze contacts, the player collided with it
	if (grazeContacts.find(bullet) != grazeContacts.end()) {
		applyGraze(1);
	}
}

void Player::applyGraze(int p)
{
	attributeSystem.modifyAttribute(Attribute::stamina, isComboActive ? p* 2 : p);
	applyCombo(p*6);
	App::playSound("sfx/graze.wav", 1.0f);
}

void Player::applyCombo(int b)
{
	attributeSystem.modifyAttribute(Attribute::combo, b);
}

void Player::startRespawn()
{
	isRespawnActive = true;
	respawnTimer = fallAnimationTime;

	suppressFiring = true;
	suppressMovement = true;
	setVel(SpaceVect::zero);
}

void Player::applyRespawn()
{
	setPos(respawnPos);
	setAngle(respawnAngle);

	space->stopAllSpriteActions(spriteID);
	space->setSpriteZoom(spriteID, zoom());

	isRespawnActive = false;
	suppressFiring = false;
	suppressMovement = false;

	respawnMaskTimer = 0.25;

	hit({ { Attribute::hp, -1.0f } }, nullptr);
}

void Player::setHudEffect(Attribute id, Attribute max_id)
{
	setHudEffect(id, getAttribute(max_id));
}

void Player::setHudEffect(Attribute id, float maxVal)
{
	float val = getAttribute(id);
	int percent = (val >= 0.0f  && maxVal > 0.0f ? val / maxVal * 100.0f : 100);

	space->addHudAction(&HUD::setPercentValue, id, percent);
}

void Player::updateHudAttribute(Attribute id)
{
	space->addHudAction(&HUD::setPercentValue, id, to_int(attributeSystem[id]));
}

bool Player::canPlaceBomb(SpaceVect pos)
{
	return !space->obstacleRadiusQuery(this, pos, 0.5, bombObstacles, PhysicsLayers::ground);
}

const AttributeMap FlandrePC::baseAttributes = {
	{Attribute::shieldLevel, 1.0f },
	{Attribute::maxHP, 100.0f},
	{Attribute::maxMP, 20.0f },
	{Attribute::maxStamina, 100.0f},
	{Attribute::staminaRegen, 5.0f},
	{Attribute::agility, 2.0f},
	{Attribute::hitProtectionInterval, 1.8f},
	{Attribute::spellCooldownInterval, 1.0f },
	{Attribute::iceSensitivity, 2.0f},
	{Attribute::sunSensitivity, 5.0f}
};

FlandrePC::FlandrePC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Player)
{}

FlandrePC::FlandrePC(GSpace* space, ObjectIDType id, const SpaceVect& pos, Direction d) :
	GObject(space, id, "player", pos, dirToPhysicsAngle(d)),
	Agent(space, id, "player", pos, d),
	Player(space, id, pos, d)
{
	multiInit.insertWithOrder(wrap_method(Player, init, this), to_int(GObject::initOrder::postLoadAttributes));

	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Player created outside of PlayScene!");
	}
}

CircleLightArea FlandrePC::getLightSource() const
{
	return CircleLightArea{
		getPos(),
		4.0,
		Color4F(.9f, .5f, .5f, .75f),
		0.5f
	};
}

void FlandrePC::setFirePattern()
{
	int level = getAttribute(Attribute::bulletCount);

	if (level == 1)
		firePattern = make_shared<FlandreFastOrbPattern>(this);
	else if (level == 3)
		firePattern = make_shared<FlandreWideAnglePattern1>(this);
	else if (level == 5)
		firePattern = make_shared<FlandreWideAnglePattern2>(this);
	
	powerAttack = make_shared<FlandreWhirlShotPattern>(this);
}

const AttributeMap RumiaPC::baseAttributes = {
	{Attribute::maxHP, 75.0f },
	{Attribute::maxMP, 25.0f },
	{Attribute::maxStamina, 75.0f },
	{Attribute::staminaRegen, 3.0f },
	{Attribute::agility, 3.0f },
	{Attribute::hitProtectionInterval, 1.2f },
	{Attribute::spellCooldownInterval, 1.0f },
	{Attribute::iceSensitivity, 1.0f },
	{Attribute::sunSensitivity, 0.0f },
	{Attribute::darknessSensitivity, 0.0f },
};

RumiaPC::RumiaPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Player)
{}

CircleLightArea RumiaPC::getLightSource() const
{
	return CircleLightArea{
		getPos(),
		3.0,
		Color4F(.5f,.2f,.5f,.5f)
	};
}

void RumiaPC::setFirePattern()
{
	firePattern = make_shared<RumiaParallelPattern>(this);
}

const AttributeMap CirnoPC::baseAttributes = {
	{Attribute::shieldLevel, 2.0f },
	{Attribute::maxHP, 125.0f },
	{Attribute::maxMP, 15.0f },
	{Attribute::maxStamina, 125.0f},
	{Attribute::staminaRegen, 6.66f },
	{Attribute::agility, 1.0f},
	{Attribute::hitProtectionInterval, 2.7f},
	{Attribute::spellCooldownInterval, 1.0f},
	{Attribute::iceSensitivity, 0.0f},
	{Attribute::sunSensitivity, 1.0f}
};

CirnoPC::CirnoPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Player)
{}

CircleLightArea CirnoPC::getLightSource() const
{
	return CircleLightArea{
		getPos(),
		5.0,
		Color4F(.4f,.45f,.7f,1.0f),
	};
}

void CirnoPC::setFirePattern()
{
	firePattern = make_shared<CirnoSmallIceBulletPattern>(this);
}
