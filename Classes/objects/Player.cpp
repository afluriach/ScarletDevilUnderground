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
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "Player.hpp"
#include "PlayerFirePattern.hpp"
#include "PlayerSpell.hpp"
#include "PlayScene.hpp"
#include "SpellDescriptor.hpp"
#include "Upgrade.hpp"

const float Player::centerLookHoldThresh = 0.3f;
const float Player::interactCooldownTime = 0.1f;
const float Player::bombCooldownTime = 1.0f;

const float Player::hitFlickerInterval = 0.333f;

const SpaceFloat Player::sprintSpeedRatio = 2.5;
const SpaceFloat Player::sprintTime = 0.5;
const SpaceFloat Player::sprintCooldownTime = 1.0;
const SpaceFloat Player::focusSpeedRatio = 0.5;
const SpaceFloat Player::bombThrowSpeed = 3.0;

const SpaceFloat Player::interactDistance = 1.25;
const SpaceFloat Player::grazeRadius = 0.7;

const float Player::bombCost = 5.0f;
const float Player::sprintCost = 7.5f;

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

	space->addGraphicsAction(&graphics_context::runSpriteAction, spriteID, pitfallShrinkAction());
	startRespawn();
}

void Player::equipFirePatterns()
{
	firePatterns.clear();

	firePatterns.push_back(make_shared<MagicMissile>(this,0));

	for (auto entry : FirePattern::playerFirePatterns)
	{
		if (space->getState()->hasItem(entry.first)) {
			shared_ptr<FirePattern> pattern = entry.second(this, 0);

			if (pattern) {
				firePatterns.push_back(pattern);
			}
		}
	}

	firePatternIdx = firePatterns.size() > 0 ? 0 : -1;
	firePattern = firePatternIdx == 0 ? firePatterns.at(0) : nullptr;

	space->addHudAction(
		&HUD::setFirePatternIcon,
		firePatternIdx == 0 ? getFirePattern()->iconPath() : ""
	);
}

void Player::equipSpells()
{
	spells.clear();

	for (string spellName : Spell::playerSpells)
	{
		if (!App::unlockAllEquips && !space->getState()->hasItem(spellName))
			continue;

		shared_ptr<SpellDesc> desc = Spell::getDescriptorByName(spellName);
		if (desc) {
			spells.push_back(desc);
		}
	}

	spellIdx = spells.size() > 0 ? 0 : -1;

	space->addHudAction(
		&HUD::setSpellIcon,
		spellIdx == 0 ? spells.at(0)->getIcon() : ""
	);
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

void Player::equipPowerAttacks()
{
	powerAttacks.clear();

	for (string spellName : Spell::playerPowerAttacks)
	{
		shared_ptr<SpellDesc> desc = Spell::getDescriptorByName(spellName);
		if (desc) {
			powerAttacks.push_back(desc);
		}
	}

	powerAttackIdx = powerAttacks.size() > 0 ? 0 : -1;

	space->addHudAction(
		&HUD::setPowerAttackIcon,
		powerAttackIdx == 0 ? powerAttacks.at(0)->getIcon() : ""
	);
}

SpaceVect Player::getInteractFeeler() const
{
	return SpaceVect::ray(interactDistance, getAngle());
}

void Player::init()
{
	if (playScene && !space->getSuppressAction()) {
		equipFirePatterns();
		equipSpells();
		equipPowerAttacks();
	}

	respawnPos = getPos();
	respawnAngle = getAngle();
}

void Player::checkMovementControls(const ControlInfo& cs)
{
	if (suppressMovement){
		return;
	}

	lookModeHoldTimer = (lookModeHoldTimer + App::secondsPerFrame)*to_int(cs.isControlActionDown(ControlAction::center_look));
	isAutoLookToggled = isAutoLookToggled && cs.isControlActionDown(ControlAction::center_look);

	if (lookModeHoldTimer >= centerLookHoldThresh && !isAutoLookToggled) {
		isAutoLook = !isAutoLook;
		isAutoLookToggled = true;
	}

	if (cs.isControlActionPressed(ControlAction::focus)) {
		space->setBulletBodiesVisible(true);
	}
	else if (cs.isControlActionReleased(ControlAction::focus)) {
		space->setBulletBodiesVisible(false);
	}

	setFocusMode(cs.isControlActionDown(ControlAction::focus) && !isSprintActive && !space->getSuppressAction());

    SpaceVect moveDir = cs.left_v;
	SpaceVect facing = isAutoLook && cs.right_v.lengthSq() < ControlRegister::deadzone2 || 
		cs.isControlActionDown(ControlAction::center_look) ?
		cs.left_v : cs.right_v;
	
	timerDecrement(sprintTimer);
	if (!isSprintActive &&
		sprintTimer <= 0.0 && 
		cs.isControlActionDown(ControlAction::sprint) &&
		!moveDir.isZero() && 
		!space->getSuppressAction() &&
		attributeSystem[Attribute::stamina] >= sprintCost
	) {
		isSprintActive = true;
		sprintTimer = sprintTime;
		attributeSystem.modifyAttribute(Attribute::stamina, -sprintCost);
		sprintDirection = moveDir.isZero() ? SpaceVect::ray(1.0, getAngle()) : moveDir.normalizeSafe();
	}
	else if (isSprintActive && sprintTimer <= 0.0) {
		isSprintActive = false;
		sprintTimer = sprintCooldownTime;
	}

	SpaceFloat speed = getMaxSpeed()*getSpeedMultiplier();
	SpaceFloat accel = getMaxAcceleration() * (isSprintActive  || !isSprintActive && sprintTimer > 0.0 ? sprintSpeedRatio * sprintSpeedRatio : 1.0);
	SpaceVect dir = isSprintActive ? sprintDirection : moveDir;

    ai::applyDesiredVelocity(this, dir*speed, accel);
    
	if (moveDir.isZero()) {
		reset();
	}
    	
	if (facing.lengthSq() > 0.0) {
		setAngle(facing.toAngle());
	}
}

void Player::updateSpellControls(const ControlInfo& cs)
{
	if (crntSpell.get() && cs.isControlActionPressed(ControlAction::spell))
	{
		stopSpell();
	}
	else if(!crntSpell.get() && spells.size() > 0 && spellIdx != -1)
    {		
		if (cs.isControlActionPressed(ControlAction::spell_previous)) {
			--spellIdx;
			if (spellIdx < 0) spellIdx += spells.size();
			log("Spell %s equipped.", spells.at(spellIdx).get()->getName().c_str());
			space->addHudAction(&HUD::setSpellIcon, spells.at(spellIdx)->getIcon());
		}
		else if (cs.isControlActionPressed(ControlAction::spell_next)) {
			++spellIdx;
			if (spellIdx >= to_int(spells.size())) spellIdx -= spells.size();
			log("Spell %s equipped.", spells.at(spellIdx).get()->getName().c_str());
			space->addHudAction(&HUD::setSpellIcon, spells.at(spellIdx)->getIcon());
		}
		
		SpellDesc* equippedSpell = nullptr;

		if (spellIdx >= 0 && spellIdx < to_int(spells.size())) {
			equippedSpell = spells.at(spellIdx).get();
		}

		if (
			!inhibitSpellcasting && 
			cs.isControlActionPressed(ControlAction::spell) &&
			equippedSpell &&
			!attributeSystem.isNonzero(Attribute::spellCooldown)
		) {
			if (cast(equippedSpell->generate(this))) {
				attributeSystem.resetCombo();
				App::playSound("sfx/player_spellcard.wav", 1.0f);
			}
		}
    }
}

void Player::onSpellStop()
{
	if (!isPowerAttack) {
		attributeSystem.setSpellCooldown();

		space->addHudAction(
			&HUD::runMagicFlicker,
			getAttribute(Attribute::spellCooldownInterval),
			hitFlickerInterval
		);
	}
	isPowerAttack = false;
}

void Player::checkFireControls(const ControlInfo& cs)
{
	bool isFireButton =
		cs.isControlActionDown(ControlAction::fire) ||
		isAutoFire && cs.right_v.lengthSq() >= ControlRegister::deadzone2
	;

	if (cs.isControlActionPressed(ControlAction::fire_mode)) {
		isAutoFire = !isAutoFire;
	}

	//Fire if arrow key is pressed
	if (!suppressFiring && isFireButton && getFirePattern()){
		float fireCost = getFirePattern()->getCost();

		if (attributeSystem[Attribute::stamina] >= fireCost && getFirePattern()->fireIfPossible()) {
			App::playSound("sfx/shot.wav", 1.0f);
			attributeSystem.modifyAttribute(Attribute::stamina, -fireCost);
		}
	}
	else if (cs.isControlActionPressed(ControlAction::fire_pattern_previous) && getFirePattern()) {
		if (firePatternIdx > 0)
			--firePatternIdx;
		else
			firePatternIdx = firePatterns.size() - 1;
		firePattern = firePatterns.at(firePatternIdx);

		space->addHudAction(&HUD::setFirePatternIcon, firePattern->iconPath());
	}
	else if (cs.isControlActionPressed(ControlAction::fire_pattern_next) && getFirePattern()) {
		++firePatternIdx;
		if (firePatternIdx >= firePatterns.size())
			firePatternIdx = 0;
		firePattern = firePatterns.at(firePatternIdx);

		space->addHudAction(&HUD::setFirePatternIcon, firePattern->iconPath());
	}
	else if (cs.isControlActionPressed(ControlAction::powerAttackNext) && powerAttackIdx != -1) {
		++powerAttackIdx;
		if (powerAttackIdx >= powerAttacks.size())
			powerAttackIdx = 0;

		space->addHudAction(&HUD::setPowerAttackIcon,powerAttacks.at(powerAttackIdx)->getIcon());
	}
	else if (
		!suppressFiring &&
		cs.isControlActionPressed(ControlAction::power_attack) &&
		powerAttackIdx != -1 &&
		!isSpellActive()
	)
	{
		if (cast(powerAttacks.at(powerAttackIdx)->generate(this))) {
			App::playSound("sfx/player_power_attack.wav", 1.0f);
			isPowerAttack = true;
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
		SpaceVect bombVel = getVel();
		if(cs.isControlActionDown(ControlAction::focus))
			bombVel += SpaceVect::ray(bombThrowSpeed, getAngle());

		if (canPlaceBomb(bombPos)) {
			space->createObject(GObject::make_object_factory<PlayerBomb>(bombPos, bombVel));
			attributeSystem.modifyAttribute(Attribute::mp, -bombCost);
			bombCooldown = bombCooldownTime;
		}
	}
}

void Player::checkItemInteraction(const ControlInfo& cs)
{
	timerDecrement(interactCooldown);

	InteractibleObject* interactible = space->interactibleObjectFeeler(this, getInteractFeeler());
	
	if(interactible && interactible->canInteract(this))
    {
        if(cs.isControlActionPressed(ControlAction::interact) && interactCooldown <= 0.0f){
            interactible->interact(this);
            interactCooldown = interactCooldownTime;
        }
    }

	space->addHudAction(
		&HUD::setInteractionIcon,
		interactible && interactible->canInteract(this) ? interactible->interactionIcon(this) : ""
	);
}

void Player::updateCombo()
{
	if (attributeSystem[Attribute::combo] >= AttributeSystem::maxComboPoints && !isComboActive) {
		isComboActive = true;
		attributeSystem.modifyAttribute(Attribute::attack, 0.25f);
		space->addGraphicsAction(&graphics_context::runSpriteAction, spriteID, comboFlickerTintAction());
	}
	else if (!attributeSystem.isNonzero(Attribute::combo) && isComboActive) {
		isComboActive = false;
		attributeSystem.modifyAttribute(Attribute::attack, -0.25f);
		space->addGraphicsAction(
			&graphics_context::stopSpriteAction,
			spriteID,
			cocos_action_tag::combo_mode_flicker
		);
		space->addGraphicsAction(&graphics_context::setSpriteColor, spriteID, Color3B::WHITE);
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
		space->addHudAction(&HUD::updateHUD, attributeSystem);

		ControlInfo cs = space->getControlInfo();

		checkMovementControls(cs);
		checkItemInteraction(cs);

		if (!space->getSuppressAction()) {
			checkFireControls(cs);
			checkBombControls(cs);
			updateSpellControls(cs);

			updateCombo();
		}

		timerDecrement(respawnTimer);
		timerDecrement(respawnMaskTimer);

		if (respawnTimer <= 0.0 && isRespawnActive) {
			applyRespawn();
		}
	}
}

SpaceFloat Player::getSpeedMultiplier()
{
	if (isSprintActive) {
		return sprintSpeedRatio;
	}
	else if (!isSprintActive && sprintTimer > sprintCooldownTime*0.5) {
		return 0.0;
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
		log("invalid attribute %d", to_int(id));
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

bool Player::hit(DamageInfo damage){
	if (!Agent::hit(damage))
		return false;

	attributeSystem.setHitProtection();
	attributeSystem.set(Attribute::combo, 0.0f);

	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
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
	return true;
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
	break;
	case Attribute::maxMP:
		attributeSystem.modifyAttribute(Attribute::mp, step);
	break;
	case Attribute::maxStamina:
		attributeSystem.modifyAttribute(Attribute::stamina, step);
	break;
	case Attribute::shieldLevel:
		space->addGraphicsAction(
			&graphics_context::setAgentOverlayShieldLevel,
			agentOverlay,
			attributeSystem[Attribute::shieldLevel]
		);
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

	space->addGraphicsAction(&graphics_context::stopAllSpriteActions, spriteID);
	space->addGraphicsAction(&graphics_context::setSpriteZoom, spriteID, zoom());

	isRespawnActive = false;
	suppressFiring = false;
	suppressMovement = false;

	respawnMaskTimer = 0.25;

	if(!space->getSuppressAction())
		hit(DamageInfo{25.0f, Attribute::end, DamageType::pitfall});
}

bool Player::canPlaceBomb(SpaceVect pos)
{
	return !space->obstacleRadiusQuery(this, pos, 0.5, bombObstacles, PhysicsLayers::ground);
}

const AttributeMap FlandrePC::baseAttributes = {
	{Attribute::shieldLevel, 1.0f },
	{Attribute::maxHP, 100.0f},
	{Attribute::maxMP, 100.0f },
	{Attribute::maxStamina, 100.0f},
	{Attribute::staminaRegen, 0.05f},
	{Attribute::agility, 2.0f},
	{Attribute::hitProtectionInterval, 1.0f},
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

const AttributeMap RumiaPC::baseAttributes = {
	{Attribute::maxHP, 75.0f },
	{Attribute::maxMP, 125.0f },
	{Attribute::maxStamina, 75.0f },
	{Attribute::staminaRegen, 0.05f },
	{Attribute::agility, 3.0f },
	{Attribute::hitProtectionInterval, 1.0f },
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

const AttributeMap CirnoPC::baseAttributes = {
	{Attribute::shieldLevel, 2.0f },
	{Attribute::maxHP, 125.0f },
	{Attribute::maxMP, 75.0f },
	{Attribute::maxStamina, 125.0f},
	{Attribute::staminaRegen, 0.05f },
	{Attribute::agility, 1.0f},
	{Attribute::hitProtectionInterval, 1.0f},
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
