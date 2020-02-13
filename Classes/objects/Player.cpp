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
#include "Collectibles.hpp"
#include "Door.hpp"
#include "FirePattern.hpp"
#include "FloorSegment.hpp"
#include "functional.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GraphicsNodes.hpp"
#include "HUD.hpp"
#include "MagicEffect.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "PlayerSpell.hpp"
#include "PlayScene.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"

const float Player::centerLookHoldThresh = 0.3f;
const float Player::interactCooldownTime = 0.1f;
const float Player::bombCooldownTime = 1.0f;
const float Player::spellCooldownTime = 1.0f;

const float Player::hitFlickerInterval = 0.333f;

const SpaceFloat Player::sprintSpeedRatio = 2.5;
const SpaceFloat Player::sprintTime = 0.5;
const SpaceFloat Player::sprintCooldownTime = 1.0;
const SpaceFloat Player::focusSpeedRatio = 0.5;
const SpaceFloat Player::bombThrowSpeed = 3.0;

const SpaceFloat Player::interactDistance = 1.25;
const SpaceFloat Player::grazeRadius = 0.7;

const float Player::sprintCost = 7.5f;

Player::Player(
	GSpace* space,
	ObjectIDType id,
	const agent_attributes& attr,
	local_shared_ptr<agent_properties> props
) :
	Agent(
		space,
		id,
		enum_bitwise_or(GType,player,canDamage),
		attr,
		props
	)
{
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

	space->addGraphicsAction(&graphics_context::runSpriteAction, spriteID, pitfallShrinkAction().generator);
	startRespawn();
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
	firePatterns.clear();

	for (auto entry : FirePattern::playerFirePatterns)
	{
		if (App::crntState->hasItem(entry.first) || app::params.unlockAllEquips) {
			local_shared_ptr<FirePattern> pattern = entry.second(this);

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
		if (!app::params.unlockAllEquips && !App::crntState->hasItem(spellName))
			continue;

		local_shared_ptr<SpellDesc> desc = Spell::getDescriptorByName(spellName);
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

void Player::equipPowerAttacks()
{
	powerAttacks.clear();

	for (string spellName : Spell::playerPowerAttacks)
	{
		local_shared_ptr<SpellDesc> desc = Spell::getDescriptorByName(spellName);
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

	respawnPos = getPos();
	respawnAngle = getAngle();
}

void Player::checkMovementControls(const ControlInfo& cs)
{
	if (suppressMovement){
		return;
	}

	if (cs.isControlActionPressed(ControlAction::focus)) {
		space->setBulletBodiesVisible(true);
	}
	else if (cs.isControlActionReleased(ControlAction::focus)) {
		space->setBulletBodiesVisible(false);
	}

	setFocusMode(cs.isControlActionDown(ControlAction::focus) && !isSprintActive && !space->getSuppressAction());

    SpaceVect moveDir = cs.left_v;
	SpaceVect facing = cs.isControlActionDown(ControlAction::center_look) ?
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
		resetAnimation();
	}
    	
	if (facing.lengthSq() > 0.0) {
		setAngle(facing.toAngle());
	}
}

void Player::updateSpellControls(const ControlInfo& cs)
{
	if (crntSpell && cs.isControlActionPressed(ControlAction::spell))
	{
		space->spellSystem->stopSpell(crntSpell);
		crntSpell = 0;
	}
	else if(!crntSpell && spells.size() > 0 && spellIdx != -1)
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
		
		local_shared_ptr<SpellDesc> equippedSpell;

		if (spellIdx >= 0 && spellIdx < to_int(spells.size())) {
			equippedSpell = spells.at(spellIdx);
		}

		if (
			!inhibitSpellcasting && 
			cs.isControlActionPressed(ControlAction::spell) &&
			equippedSpell &&
			spellCooldown <= 0.0f
		) {
			crntSpell = space->spellSystem->cast(equippedSpell, this);
			if (crntSpell) {
				attributeSystem.resetCombo();
				playSoundSpatial("sfx/player_spellcard.wav");
			}
		}
    }
}

void Player::onSpellStop()
{
	if (!isPowerAttack) {
		spellCooldown = spellCooldownTime;

		space->addHudAction(
			&HUD::runMagicFlicker,
			spellCooldownTime,
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
			playSoundSpatial("sfx/shot.wav", 1.0f);
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
		!crntSpell
	)
	{
		if ( space->spellSystem->cast(powerAttacks.at(powerAttackIdx), this)) {
			playSoundSpatial("sfx/player_power_attack.wav");
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
		crntBomb &&
		attributeSystem[Attribute::mp] >= crntBomb->cost)
	{
		SpaceVect bombPos = getPos() + SpaceVect::ray(1.5, getAngle());
		SpaceVect bombVel = getVel();
		if(cs.isControlActionDown(ControlAction::focus))
			bombVel += SpaceVect::ray(bombThrowSpeed, getAngle());

		if (canPlaceBomb(bombPos)) {
			space->createObject<Bomb>(
				object_params(bombPos,bombVel),
				crntBomb
			);
			attributeSystem.modifyAttribute(Attribute::mp, crntBomb->cost);
			bombCooldown = bombCooldownTime;
		}
	}
}

void Player::checkItemInteraction(const ControlInfo& cs)
{
	timerDecrement(interactCooldown);

	GObject* interactible = space->physicsContext->interactibleObjectFeeler(
		this,
		getInteractFeeler()
	);
	
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
		space->addGraphicsAction(&graphics_context::runSpriteAction, spriteID, comboFlickerTintAction().generator);
	}
	else if (!attributeSystem.isNonzero(Attribute::combo) && isComboActive) {
		isComboActive = false;
		attributeSystem.modifyAttribute(Attribute::attack, -0.25f);
		space->addGraphicsAction(
			&graphics_context::stopSpriteAction,
			spriteID,
			cocos_action_tag::combo_mode_flicker
		);
		space->graphicsNodeAction(&Node::setColor, spriteID, Color3B::WHITE);
	}

	if (attributeSystem[Attribute::combo] > 0) {
		attributeSystem.modifyAttribute(Attribute::combo, -app::params.secondsPerFrame * 15.0f);
	}
}

void Player::onZeroHP()
{
	if (!GScene::suppressGameOver) {
		space->audioContext->playSound("sfx/player_death.wav", 0.5f);

		space->addSceneAction(
			[=]()->void { playScene->triggerGameOver(); }
		);
	}
}

void Player::update()
{
	Agent::update();

	space->audioContext->setSoundListenerPos(getPos(), getVel(), float_pi/2.0);

	if (playScene) {
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

ALuint Player::playSoundSpatial(const string& path, float volume, bool loop, float yPos)
{
	ALuint soundSource = space->audioContext->playSound(
		path,
		volume,
		loop
	);

	return soundSource;
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
	return App::crntState->attributeUpgrades;
}

void Player::onBulletCollide(Bullet* b, SpaceVect n)
{
	grazeContacts.erase(b);
	Agent::onBulletCollide(b, n);
}

void Player::onBulletHitTarget(Bullet* bullet, Agent* target)
{
	applyCombo(6);
}

bool Player::hit(DamageInfo damage, SpaceVect n){
	if (!Agent::hit(damage, n))
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
		).generator
	);

	space->addHudAction(
		&HUD::runHealthFlicker,
		attributeSystem[Attribute::hitProtectionInterval],
		hitFlickerInterval
	);

	playSoundSpatial("sfx/player_damage.wav");
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

void Player::applyUpgrade(Attribute attr, float val)
{
	attributeSystem.modifyAttribute(attr, val);

	if (attr == Attribute::shieldLevel) {
		space->graphicsNodeAction(
			&AgentBodyShader::setShieldLevel,
			agentOverlay,
			attributeSystem[Attribute::shieldLevel]
		);
	}

	App::crntState->applyAttributeUpgrade(attr, val);
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
	playSoundSpatial("sfx/graze.wav");
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
	space->addGraphicsAction(
		&graphics_context::setSpriteZoom,
		spriteID,
		getSpriteZoom(app::getSprite(getSprite()), getRadius())
	);

	isRespawnActive = false;
	suppressFiring = false;
	suppressMovement = false;

	respawnMaskTimer = 0.25;

	if(!space->getSuppressAction())
		hit(DamageInfo(25.0f, DamageType::pitfall), SpaceVect::zero);
}

bool Player::canPlaceBomb(SpaceVect pos)
{
	return !space->physicsContext->obstacleRadiusQuery(
		this,
		pos,
		0.5,
		obstacles,
		PhysicsLayers::ground
	);
}
