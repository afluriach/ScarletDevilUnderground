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
#include "Bullet.hpp"
#include "Collectibles.hpp"
#include "controls.h"
#include "FirePattern.hpp"
#include "FloorSegment.hpp"
#include "functional.hpp"
#include "GAnimation.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

const float Player::interactCooldownTime = 0.1f;
const float Player::hitFlickerInterval = 0.333f;

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

void Player::init()
{
	if (playScene && !playScene->getSuppressAction()) {
		space->getScene()->addAction(make_hud_action(
			&HUD::setMaxHP,
			to_int(attributeSystem.getAdjustedValue(Attribute::maxHP))
		));

		space->getScene()->addAction(make_hud_action(
			&HUD::setHP,
			to_int(attributeSystem.getAdjustedValue(Attribute::hp))
		));

		setFirePatterns();

		if (getFirePattern()) {
			space->getScene()->addAction(make_hud_action(
				&HUD::setFirePatternIcon,
				getFirePattern()->iconPath()
			));
		}

		equipSpells();
	}
}

void Player::checkMovementControls(const ControlInfo& cs)
{
    SpaceVect moveDir = cs.left_v;
	SpaceVect facing = cs.right_v;

    ai::applyDesiredVelocity(this, moveDir*getMaxSpeed(), getMaxAcceleration());
    
    if(moveDir.isZero())
         animSprite->reset();
    
    //Facing is not diagonal, horizontal direction will override.
    setDirection(toDirection(facing));
}

void Player::updateSpellControls(const ControlInfo& cs)
{
    if(crntSpell.get())
    {
        if(cs.isControlActionPressed(ControlAction::spell1)){
            stopSpell();
        }
    }
    else
    {
		attributeSystem.timerDecrement(Attribute::spellCooldown);
        
        if(!attributeSystem.isNonzero(Attribute::spellCooldown) && equippedSpell){
            if( cs.isControlActionPressed(ControlAction::spell1) &&
                attributeSystem.getAdjustedValue(Attribute::power) >= equippedSpell->getInitialCost()){
				cast(equippedSpell->generate(this, {}));
            }
        }
    }
}

void Player::onSpellStop()
{
	attributeSystem.setSpellCooldown();

	space->getScene()->addAction(make_hud_action(
		&HUD::runPowerFlicker,
		getAttribute(Attribute::spellCooldownInterval)
	));
}

void Player::checkFireControls(const ControlInfo& cs)
{
	//Check controls for changing fire pattern.
	if (!suppressFiring && getFirePattern() && !getFirePattern()->isInCooldown())
	{
		if (cs.isControlActionPressed(ControlAction::firePatternPrev))
			trySetFirePatternPrevious();
		else if (cs.isControlActionPressed(ControlAction::firePatternNext))
			trySetFirePatternNext();
	}

	//Fire if arrow key is pressed
	if (!suppressFiring && !cs.right_v.isZero() && getFirePattern())
	{
		getFirePattern()->fireIfPossible();
	}

}

void Player::checkItemInteraction(const ControlInfo& cs)
{
	timerDecrement(interactCooldown);
	GObject* item = getSensedObject();
	InteractibleObject* interactible = dynamic_cast<InteractibleObject*>(item);
	
	if(interactible && interactible->canInteract())
    {
        if(cs.isControlActionPressed(ControlAction::interact) && interactCooldown <= 0.0f){
            interactible->interact();
            interactCooldown = interactCooldownTime;
        }
    }

	space->getScene()->addAction(make_hud_action(
		&HUD::setInteractionIcon,
		interactible && interactible->canInteract() ? interactible->interactionIcon() : ""
	));
}

void Player::updateHitTime()
{
	attributeSystem.timerDecrement(Attribute::hitProtection);
}

void Player::onZeroHP()
{
	if(!GScene::suppressGameOver)
		playScene->triggerGameOver();
}

void Player::update()
{
	if (playScene) {
		ControlInfo cs = playScene->getControlData();

		checkMovementControls(cs);
		checkItemInteraction(cs);

		if (!playScene->getSuppressAction()) {
			checkFireControls(cs);
			updateSpellControls(cs);

			if (getFirePattern())
				getFirePattern()->update();

			updateHitTime();

			setHudEffect(Attribute::hitProtection, Attribute::hitProtectionInterval);
			setHudEffect(Attribute::spellCooldown, Attribute::spellCooldownInterval);

			updateHudAttribute(Attribute::iceDamage);
			updateHudAttribute(Attribute::sunDamage);
			updateHudAttribute(Attribute::poisonDamage);
			updateHudAttribute(Attribute::slimeDamage);
		}
	}
}

void Player::applyAttributeModifier(Attribute id, float val)
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return;
	}

	attributeSystem.modifyAttribute(id, val);
}

FirePattern* Player::getFirePattern()
{
	if (firePatterns.empty())
		return nullptr;

	return firePatterns[crntFirePattern].get();
}

bool Player::isProtected() const
{
	return attributeSystem.getAdjustedValue(Attribute::hitProtection) != 0.0f;
}

void Player::setProtection()
{
	attributeSystem.setProtection();
}

void Player::resetProtection()
{
	attributeSystem.resetProtection();;
}


void Player::hit(AttributeMap attributeEffect, shared_ptr<MagicEffect> effect){
    if(!isProtected()){
		Agent::hit(attributeEffect, effect);
		attributeSystem.setHitProtection();

        sprite->runAction(
			flickerAction(
				hitFlickerInterval,
				attributeSystem.getAdjustedValue(Attribute::hitProtectionInterval),
				81
			)
		);

		space->getScene()->addAction(make_hud_action(
			&HUD::runHealthFlicker,
			attributeSystem.getAdjustedValue(Attribute::hitProtectionInterval),
			hitFlickerInterval
		));
    }
}

void Player::onCollectible(Collectible* coll)
{
	applyAttributeEffects(coll->getEffect());
    space->removeObject(coll);
}

void Player::applyGraze(int p)
{
	attributeSystem.modifyAttribute(Attribute::power, p);
}


bool Player::trySetFirePattern(int idx)
{
	FirePattern* fp = getFirePattern();

	if (fp->isInCooldown())
		return false;

	if (idx < 0 || idx >= firePatterns.size()) {
		log("trySetFirePattern: invalid index %d", idx);
		return false;
	}
	else {
		crntFirePattern = idx;

		space->getScene()->addAction(make_hud_action(
			&HUD::setFirePatternIcon,
			getFirePattern()->iconPath()
		));

		return true;
	}
}

bool Player::trySetFirePatternNext()
{
	return trySetFirePattern((crntFirePattern + 1) % firePatterns.size());
}

bool Player::trySetFirePatternPrevious()
{
	return trySetFirePattern((crntFirePattern - 1) % firePatterns.size());
}

void Player::setHudEffect(Attribute id, Attribute max_id)
{
	float val = getAttribute(id);
	float maxVal = getAttribute(max_id);
	int percent = (val >= 0.0f  && maxVal > 0.0f ? val / maxVal * 100.0f : 100);

	space->getScene()->addAction(make_hud_action(
		&HUD::setPercentValue,
		id,
		percent
	));
}

void Player::updateHudAttribute(Attribute id)
{
	space->getScene()->addAction(make_hud_action(
		&HUD::setPercentValue,
		id,
		static_cast<int>(attributeSystem.getAdjustedValue(id))
	));
}

const AttributeMap FlandrePC::baseAttributes = {
	{Attribute::maxHP, 5.0f},
	{Attribute::maxMP, 5.0f },
	{Attribute::maxPower, 500.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 9.0f},
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

void FlandrePC::setFirePatterns()
{
	firePatterns.push_back(make_unique<FlandreBigOrbPattern>(this));
	firePatterns.push_back(make_unique<FlandreFastOrbPattern>(this));
}

void FlandrePC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerBatMode")->second.get();
}

const AttributeMap RumiaPC::baseAttributes = {
	{Attribute::maxHP, 3.0f },
	{Attribute::maxMP, 3.0f },
	{Attribute::maxPower, 900.0f },
	{Attribute::speed, 4.5f },
	{Attribute::acceleration, 12.0f },
	{Attribute::hitProtectionInterval, 1.5f },
	{ Attribute::spellCooldownInterval, 1.0f },
	{Attribute::iceSensitivity, 1.0f },
	{Attribute::sunSensitivity, 0.0f }
};

RumiaPC::RumiaPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Player)
{}

void RumiaPC::setFirePatterns()
{
	firePatterns.push_back(make_unique<RumiaFastOrbPattern>(this));
}

void RumiaPC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerDarkMist")->second.get();
}

const AttributeMap CirnoPC::baseAttributes = {
	{Attribute::maxHP, 9.0f },
	{Attribute::maxMP, 3.0f },
	{Attribute::maxPower, 300.0f},
	{Attribute::speed, 2.0f},
	{Attribute::acceleration, 6.0f},
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

void CirnoPC::setFirePatterns()
{
	firePatterns.push_back(make_unique<CirnoLargeIceBulletPattern>(this));
}

void CirnoPC::equipSpells() {
	//NO-OP
}
