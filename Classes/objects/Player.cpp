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

const boost::rational<int> Player::interactCooldownTime = boost::rational<int>(1,10);
const boost::rational<int> Player::spellCooldownTime = 1;
const boost::rational<int> Player::hitFlickerInterval = boost::rational<int>(1,3);

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
	setFirePatterns();

	if (playScene) {
		space->getScene()->addAction(make_hud_action(
			&HUD::setMaxHP,
			to_int(attributeSystem.getAdjustedValue(Attribute::maxHP))
		));

		space->getScene()->addAction(make_hud_action(
			&HUD::setHP,
			to_int(attributeSystem.getAdjustedValue(Attribute::hp))
		));

		if (getFirePattern()) {
			space->getScene()->addAction(make_hud_action(
				&HUD::setFirePatternIcon,
				getFirePattern()->iconPath()
			));
		}
	}

	equipSpells();
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
		timerDecrement(spellCooldown);
        
        if(spellCooldown <= 0 && equippedSpell){
            if( cs.isControlActionPressed(ControlAction::spell1) &&
                attributeSystem.getAdjustedValue(Attribute::power) >= equippedSpell->getInitialCost()){
				cast(equippedSpell->generate(this, {}));
            }
        }
    }
}

void Player::onSpellStop()
{
    spellCooldown = spellCooldownTime;

	space->getScene()->addAction(
		bind(
			&PowerMeter::runFlicker,
			playScene->hud->power
		),
		GScene::updateOrder::hudUpdate
	);
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
    if(hitProtectionCountdown > 0)
    {
        hitProtectionCountdown -= App::secondsPerFrameRational;
        
        if(hitProtectionCountdown <= 0){
            hitProtectionCountdown = 0;
        }
    }
}

void Player::onZeroHP()
{
	if(!GScene::suppressGameOver)
		playScene->triggerGameOver();
}

void Player::update()
{
	if(getFirePattern())
		getFirePattern()->update();

	if (playScene) {
		ControlInfo cs = playScene->getControlData();

		checkMovementControls(cs);
		checkFireControls(cs);
		updateSpellControls(cs);
		checkItemInteraction(cs);

		space->getScene()->addAction(make_hud_action(
			&HUD::setIceDamage,
			attributeSystem.getAdjustedValue(Attribute::iceDamage) / 25.0f
		));

		space->getScene()->addAction(make_hud_action(
			&HUD::setSunDamage,
			attributeSystem.getAdjustedValue(Attribute::sunDamage) / 25.0f
		));
	}

	updateHitTime();    
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

void Player::hit(AttributeMap attributeEffect, shared_ptr<MagicEffect> effect){
    if(hitProtectionCountdown <= 0 && !spellProtectionMode){

        hitProtectionCountdown = attributeSystem.getAdjustedValue(Attribute::hitProtection);

        sprite->runAction(
			flickerAction(
				boost::rational_cast<float>(hitFlickerInterval),
				boost::rational_cast<float>(hitProtectionCountdown),
				81
			)
		);

		space->getScene()->addAction(make_hud_action(
			&HUD::runHealthFlicker,
			boost::rational_cast<float>(hitProtectionCountdown),
			boost::rational_cast<float>(hitFlickerInterval)
		));

		Agent::hit(attributeEffect, effect);
    }
}

void Player::onCollectible(Collectible* coll)
{
    PowerUp* p = dynamic_cast<PowerUp*>(coll);
    
    if(p){
		attributeSystem.modifyAttribute(Attribute::power, 10);
                
        space->removeObject(coll);
    }
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

const AttributeMap FlandrePC::baseAttributes = {
	{Attribute::maxHP, 5.0f},
	{Attribute::maxPower, 500.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 9.0f},
	{Attribute::hitProtection, 2.4f},
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
	{Attribute::maxPower, 900.0f },
	{Attribute::speed, 4.5f },
	{Attribute::acceleration, 12.0f },
	{Attribute::hitProtection, 1.5f },
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
}

void RumiaPC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerDarkMist")->second.get();
}

const AttributeMap CirnoPC::baseAttributes = {
	{Attribute::maxHP, 9.0f },
	{Attribute::maxPower, 300.0f},
	{Attribute::speed, 2.0f},
	{Attribute::acceleration, 6.0f},
	{Attribute::hitProtection, 3.3f},
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
