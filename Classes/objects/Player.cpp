//
//  Player.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "App.h"
#include "Bomb.hpp"
#include "Collectibles.hpp"
#include "controls.h"
#include "Door.hpp"
#include "EnemyBullet.hpp"
#include "FloorSegment.hpp"
#include "functional.hpp"
#include "GAnimation.hpp"
#include "Graphics.h"
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

const float Player::bombPowerCost = 25.0f;
const float Player::powerAttackCost = 25.0f;

const GType Player::bombObstacles = enum_bitwise_or4(GType, enemy, environment, wall, bomb);

Player::Player(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(Agent),
	RegisterUpdate<Player>(this)
{
	multiInit.insertWithOrder(wrap_method(Player, init, this), to_int(GObject::initOrder::postLoadAttributes));

	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Player created outside of PlayScene!");
	}
}

void Player::initializeGraphics()
{
	PatchConSprite::initializeGraphics();

	drawNodeID = space->createDrawNode(GraphicsLayer::agentOverlay, getInitialCenterPix(), 1.0f);
	light = space->addLightSource(getLight());

	space->drawSolidCircle(drawNodeID, Vec2::ZERO, to_float(App::pixelsPerTile*grazeRadius), 0.0f, 64, Color4F(0.5f, 0.5f, 0.5f, 0.5f));
	space->drawSolidCircle(drawNodeID, Vec2::ZERO, to_float(App::pixelsPerTile*getRadius()), 0.0f, 64, Color4F(0.5f, 0.5f, 0.5f, 0.5f));
}

void Player::onPitfall()
{
	if (isRespawnActive || respawnMaskTimer > 0.0) {
		return;
	}

	space->runSpriteAction(spriteID, pitfallShrinkAction());
	startRespawn();
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
		space->addSceneAction(make_hud_action(
			&HUD::setMaxHP,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::maxHP))
		));

		space->addSceneAction(make_hud_action(
			&HUD::setHP,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::hp))
		));

		space->addSceneAction(make_hud_action(
			&HUD::setMaxMP,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::maxMP))
		));

		space->addSceneAction(make_hud_action(
			&HUD::setMP,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::mp))
		));

		space->addSceneAction(make_hud_action(
			&HUD::setPower,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::power))
		));

		setFirePatterns();

		if (getFirePattern()) {
			space->addSceneAction(make_hud_action(
				&HUD::setFirePatternIcon,
				playScene,
				getFirePattern()->iconPath()
			));
		}

		equipSpells();

		respawnPos = getPos();
		respawnAngle = getAngle();
	}
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
    if(!crntSpell.get())
    {
		attributeSystem.timerDecrement(Attribute::spellCooldown);
        
		if (
			cs.isControlActionPressed(ControlAction::spell) &&
			equippedSpell &&
			!attributeSystem.isNonzero(Attribute::spellCooldown) &&
			attributeSystem.isNonzero(Attribute::mp)
		) {
			attributeSystem.modifyAttribute(Attribute::mp, -1.0f);
			cast(equippedSpell->generate(this));		
			attributeSystem.resetCombo();
			App::playSound("sfx/player_spellcard.wav", 1.0f);
		}
    }
}

void Player::onSpellStop()
{
	attributeSystem.setSpellCooldown();

	space->addSceneAction(make_hud_action(
		&HUD::runMagicFlicker,
		playScene,
		getAttribute(Attribute::spellCooldownInterval),
		hitFlickerInterval
	));
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
		getPower() >= powerAttackCost)
	{
		if (powerAttack->fireIfPossible()) {
			App::playSound("sfx/shot.wav", 1.0f);
			attributeSystem.modifyAttribute(Attribute::power, -powerAttackCost);
		}
	}
}

void Player::checkBombControls(const ControlInfo& cs)
{
	timerDecrement(bombCooldown);

	if (!suppressFiring &&
		cs.isControlActionPressed(ControlAction::bomb) &&
		bombCooldown <= 0.0f &&
		attributeSystem.getAdjustedValue(Attribute::power) >= bombPowerCost)
	{
		SpaceVect bombPos = getPos() + SpaceVect::ray(1.5, getAngle());
		if (canPlaceBomb(bombPos)) {
			space->createObject(GObject::make_object_factory<PlayerBomb>(bombPos, getVel()));
			attributeSystem.modifyAttribute(Attribute::power, -bombPowerCost);
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

	space->addSceneAction(make_hud_action(
		&HUD::setInteractionIcon,
		playScene,
		interactible && interactible->canInteract() ? interactible->interactionIcon() : ""
	));
}

void Player::updateHitTime()
{
	attributeSystem.timerDecrement(Attribute::hitProtection);
}

void Player::updateCombo()
{
	if (attributeSystem.getAdjustedValue(Attribute::combo) >= AttributeSystem::maxComboPoints && !isComboActive) {
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

	if (attributeSystem.getAdjustedValue(Attribute::combo) > 0) {
		attributeSystem.modifyAttribute(Attribute::combo, -App::secondsPerFrame * 15.0f);
	}
}

void Player::onZeroHP()
{
	if (!GScene::suppressGameOver) {
		App::playSound("sfx/player_death.wav", 0.5f);

		space->addSceneAction(
			[=]()->void { playScene->triggerGameOver();},
			GScene::updateOrder::sceneUpdate
		);
	}
}

void Player::update()
{
	App::setSoundListenerPos(getPos(), getVel(), float_pi/2.0);

	if (playScene) {
		space->updatePlayerMapLocation(getPos());

		ControlInfo cs = playScene->getControlData();

		checkMovementControls(cs);
		checkItemInteraction(cs);

		if (!space->getSuppressAction()) {
			checkFireControls(cs);
			checkBombControls(cs);
			updateSpellControls(cs);

			updateHitTime();
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

			space->addSceneAction(make_hud_action(
				&HUD::setHP,
				playScene,
				to_int(attributeSystem.getAdjustedValue(Attribute::hp))
			));
			space->addSceneAction(make_hud_action(
				&HUD::setMP,
				playScene,
				to_int(attributeSystem.getAdjustedValue(Attribute::mp))
			));
			space->addSceneAction(make_hud_action(
				&HUD::setPower,
				playScene,
				to_int(attributeSystem.getAdjustedValue(Attribute::power))
			));
			space->addSceneAction(make_hud_action(
				&HUD::setKeyCount,
				playScene,
				to_int(attributeSystem.getAdjustedValue(Attribute::keys))
			));
		}

		timerDecrement(respawnTimer);
		timerDecrement(respawnMaskTimer);

		if (respawnTimer <= 0.0 && isRespawnActive) {
			applyRespawn();
		}

		if (powerAttack) {
			powerAttack->update();
		}

		space->setLightSourcePosition(light, getPos());
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
	space->setSpriteVisible(drawNodeID, b);
}

void Player::setSprintMode(bool b)
{
	isSprintActive = b;
}

unsigned int Player::getKeyCount() const
{
	return attributeSystem.getAdjustedValue(Attribute::keys);
}

void Player::useKey()
{
	if (getKeyCount() > 0) {
		attributeSystem.modifyAttribute(Attribute::keys, -1.0f);
	}
}

AttributeMap Player::getAttributeUpgrades() const
{
	AttributeMap result;

	//static const array<Attribute, AttributeSystem::upgradeAttributesCount> upgradeAttributes;

	for (size_t upgradeIndex = 0; upgradeIndex < AttributeSystem::upgradeAttributesCount; ++upgradeIndex)
	{
		Attribute at = AttributeSystem::upgradeAttributes[upgradeIndex];

		result.insert_or_assign(at, App::crntState->getUpgradeLevel(App::crntPC, at));
	}

	return result;
}

void Player::applyAttributeModifier(Attribute id, float val)
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return;
	}

	attributeSystem.modifyAttribute(id, val);
}

bool Player::isProtected() const
{
	return attributeSystem.getAdjustedValue(Attribute::hitProtection) != 0.0f;
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

void Player::hit(AttributeMap attributeEffect, shared_ptr<MagicEffect> effect){
    if(!isProtected()){
		Agent::hit(attributeEffect, effect);
		attributeSystem.setHitProtection();
		attributeSystem.setAttribute(Attribute::combo, 0.0f);

		space->runSpriteAction(
			spriteID,
			flickerAction(
				hitFlickerInterval,
				attributeSystem.getAdjustedValue(Attribute::hitProtectionInterval),
				81
			)
		);

		space->addSceneAction(make_hud_action(
			&HUD::runHealthFlicker,
			playScene,
			attributeSystem.getAdjustedValue(Attribute::hitProtectionInterval),
			hitFlickerInterval
		));

		App::playSound("sfx/player_damage.wav", 1.0f);
    }
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
	Door* dest = interactTarget->getAdjacent();

	if (dest)
	{
		moveToDestinationDoor(dest);
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
	attributeSystem.modifyAttribute(at, 1.0f);

	switch (at)
	{
	case Attribute::maxHP:
		attributeSystem.modifyAttribute(Attribute::hp, 1.0f);
		space->addSceneAction(make_hud_action(
			&HUD::setMaxHP,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::maxHP))
		));
	break;
	case Attribute::maxMP:
		attributeSystem.modifyAttribute(Attribute::mp, 1.0f);
		space->addSceneAction(make_hud_action(
			&HUD::setMaxMP,
			playScene,
			to_int(attributeSystem.getAdjustedValue(Attribute::maxMP))
		));
	break;
	case Attribute::maxPower:
		attributeSystem.modifyAttribute(Attribute::power, 0.5f);
	break;
	}

	App::crntState->registerUpgrade(App::crntPC, at, up->upgrade_id);
	space->removeObject(up);
}

void Player::onGrazeTouch(object_ref<EnemyBullet> bullet)
{
	if (bullet.isValid() && bullet.get()->grazeValid) {
		grazeContacts.insert(bullet);
	}
}

//Effect is applied after the graze "radar" loses contact.
void Player::onGrazeCleared(object_ref<EnemyBullet> bullet)
{
	if (bullet.isValid() && bullet.get()->grazeValid) {
		grazeContacts.erase(bullet);
		applyGraze(1);
	}
}

void Player::applyGraze(int p)
{
	attributeSystem.modifyAttribute(Attribute::power, isComboActive ? p* 2 : p);
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
	float val = getAttribute(id);
	float maxVal = getAttribute(max_id);
	int percent = (val >= 0.0f  && maxVal > 0.0f ? val / maxVal * 100.0f : 100);

	space->addSceneAction(make_hud_action(
		&HUD::setPercentValue,
		playScene,
		id,
		percent
	));
}

void Player::setHudEffect(Attribute id, float maxVal)
{
	float val = getAttribute(id);
	int percent = (val >= 0.0f  && maxVal > 0.0f ? val / maxVal * 100.0f : 100);

	space->addSceneAction(make_hud_action(
		&HUD::setPercentValue,
		playScene,
		id,
		percent
	));
}

void Player::updateHudAttribute(Attribute id)
{
	space->addSceneAction(make_hud_action(
		&HUD::setPercentValue,
		playScene,
		id,
		to_int(attributeSystem.getAdjustedValue(id))
	));
}

bool Player::canPlaceBomb(SpaceVect pos)
{
	return !space->obstacleRadiusQuery(this, pos, 0.5, bombObstacles, PhysicsLayers::ground);
}

const AttributeMap FlandrePC::baseAttributes = {
	{Attribute::maxHP, 8.0f},
	{Attribute::maxMP, 4.0f },
	{Attribute::maxPower, 200.0f},
	{Attribute::agility, 2.0f},
	{Attribute::hitProtectionInterval, 2.4f},
	{Attribute::spellCooldownInterval, 1.0f },
	{Attribute::iceSensitivity, 2.0f},
	{Attribute::sunSensitivity, 5.0f}
};

FlandrePC::FlandrePC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Player)
{}

CircleLightArea FlandrePC::getLight()
{
	return CircleLightArea{
		getPos(),
		4.0,
		Color4F(.9f, .5f, .5f, .75f),
		0.5f
	};
}

void FlandrePC::setFirePatterns()
{
	firePattern = make_shared<FlandreWideAnglePattern1>(this);
	powerAttack = make_shared<FlandreWhirlShotPattern>(this);
}

void FlandrePC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerBatMode")->second.get();
}

const AttributeMap RumiaPC::baseAttributes = {
	{Attribute::maxHP, 6.0f },
	{Attribute::maxMP, 4.0f },
	{Attribute::maxPower, 300.0f },
	{Attribute::agility, 3.0f },
	{Attribute::hitProtectionInterval, 1.5f },
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

CircleLightArea RumiaPC::getLight()
{
	return CircleLightArea{
		getPos(),
		3.0,
		Color4F(.5f,.2f,.5f,.5f)
	};
}

void RumiaPC::setFirePatterns()
{
	firePattern = make_shared<RumiaParallelPattern>(this);
}

void RumiaPC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerDarkMist")->second.get();
}

const AttributeMap CirnoPC::baseAttributes = {
	{Attribute::maxHP, 10.0f },
	{Attribute::maxMP, 4.0f },
	{Attribute::maxPower, 200.0f},
	{Attribute::agility, 1.0f},
	{Attribute::hitProtectionInterval, 3.3f},
	{Attribute::spellCooldownInterval, 1.0f},
	{Attribute::iceSensitivity, 0.0f},
	{Attribute::sunSensitivity, 1.0f}
};

CirnoPC::CirnoPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Player)
{}

CircleLightArea CirnoPC::getLight()
{
	return CircleLightArea{
		getPos(),
		5.0,
		Color4F(.4f,.45f,.7f,1.0f),
	};
}

void CirnoPC::setFirePatterns()
{
	firePattern = make_shared<CirnoSmallIceBulletPattern>(this);
}

void CirnoPC::equipSpells() {
	//NO-OP
	equippedSpell = Spell::spellDescriptors.find("PlayerIceShield")->second.get();
}
