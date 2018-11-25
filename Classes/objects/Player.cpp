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
#include "GAnimation.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

const float Player::interactCooldownTime = 0.1f;

const float Player::spellCooldownTime = 1.0f;
const float Player::hitFlickerInterval = 0.3f;

Player::Player(const ValueMap& args) :
	GObject(args),
	Agent(args),
	RegisterInit<Player>(this),
	RegisterUpdate<Player>(this)
{}

void Player::init()
{
	setFirePatterns();
	if(getFirePattern())
		app->hud->firePatternIcon->setTexture(getFirePattern()->iconPath());

	attributeSystem.baseAttributes = getAttributes();

	power = attributeSystem.getAdjustedValue(Attribute::power);
	health = attributeSystem.getAdjustedValue(Attribute::health);

	app->hud->health->setMax(health);

	equipSpells();
}

void Player::checkMovementControls()
{
    auto cr = app->control_register;
    
    SpaceVect moveDir = cr->getLeftVector();
    
    ai::applyDesiredVelocity(*this, moveDir*getMaxSpeed(), getMaxAcceleration());
    
    if(moveDir.isZero())
         animSprite->reset();
    
    SpaceVect facing = cr->getRightVector();
    
    //Facing is not diagonal, horizontal direction will override.
    setDirection(toDirection(facing));
    
    //Player will automatically face their movement direction if look keys are not pressed
    if(facing.isZero() && body->getVel().lengthSq() > square(getMaxSpeed())/2){
        setDirection(toDirection(moveDir));
    }

}

void Player::updateSpellControls()
{
    auto cr = app->control_register;
    
    if(crntSpell.get())
    {
        if(cr->isControlActionPressed(ControlAction::spell1)){
            stopSpell();
        }
    }
    else
    {
        spellCooldown = max(spellCooldown - App::secondsPerFrame, 0.0f);
        
        if(spellCooldown <= 0){
            if( cr->isControlActionPressed(ControlAction::spell1) &&
                power > equippedSpell->getInitialCost()){
				cast(equippedSpell->generate(this, {}));
            }
        }
    }
}

void Player::onSpellStop()
{
    spellCooldown = spellCooldownTime;
    app->hud->power->runFlicker();
}

void Player::checkFireControls()
{
	auto cr = app->control_register;

	//Check controls for changing fire pattern.
	if (!suppressFiring && getFirePattern() && !getFirePattern()->isInCooldown())
	{
		if (cr->isControlActionPressed(ControlAction::firePatternPrev))
			trySetFirePatternPrevious();
		else if (cr->isControlActionPressed(ControlAction::firePatternNext))
			trySetFirePatternNext();
	}

	//Fire if arrow key is pressed
	if (!suppressFiring && !cr->getRightVector().isZero() && getFirePattern())
	{
		getFirePattern()->fireIfPossible();
	}

}

void Player::checkItemInteraction()
{
	interactCooldown = max(interactCooldown - App::secondsPerFrame, 0.0f);
	GObject* item = getSensedObject();
	InteractibleObject* interactible = dynamic_cast<InteractibleObject*>(item);
	
	if(interactible && interactible->canInteract())
    {
        app->hud->setInteractionIcon(interactible->interactionIcon());

        if(app->control_register->isControlActionPressed(ControlAction::interact) && interactCooldown <= 0.0f){
            interactible->interact();
            interactCooldown = interactCooldownTime;
        }
    }
    else
    {
        app->hud->setInteractionIcon("");
    }
}

void Player::updateHitTime()
{
    if(hitProtectionCountdown > 0)
    {
        hitProtectionCountdown -= App::secondsPerFrame;
        
        if(hitProtectionCountdown < 0){
            sprite->setOpacity(255);
            hitProtectionCountdown = 0;
        }
    }
}

void Player::update()
{
	if(getFirePattern())
		getFirePattern()->update();

	checkMovementControls();
	checkFireControls();
	updateSpellControls();

	updateHitTime();
    
    if(health <= 0 && !app->suppressGameOver)
        app->playScene->triggerGameOver();
}

void Player::applyAttributeModifier(Attribute id, float val)
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return;
	}

	attributeSystem.modifiers.at(to_size_t(id)) += val;
}

void Player::hit(){
    if(hitProtectionCountdown <= 0 && !spellProtectionMode){
        hitProtectionCountdown = attributeSystem.getAdjustedValue(Attribute::hitProtection);
        sprite->runAction(flickerAction(hitFlickerInterval, hitProtectionCountdown, 81.0f));
        app->hud->health->runFlicker(hitProtectionCountdown);
    
        health -= 1;
        if(health < 0) health = 0;
    }
}

float Player::getMaxSpeed() const
{
	return attributeSystem.getAdjustedValue(Attribute::speed);
}

float Player::getMaxAcceleration() const
{
	return attributeSystem.getAdjustedValue(Attribute::acceleration);
}

float Player::getMaxPower() const
{
	return attributeSystem.getAdjustedValue(Attribute::power);
}

float Player::getMaxHealth() const
{
	return attributeSystem.getAdjustedValue(Attribute::health);
}


void Player::onCollectible(Collectible* coll)
{
    PowerUp* p = dynamic_cast<PowerUp*>(coll);
    
    if(p){
        power += 10;
        
        if(power > attributeSystem.getAdjustedValue(Attribute::power))
            power = attributeSystem.getAdjustedValue(Attribute::power);
        
        app->space->removeObject(coll);
    }
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
		app->hud->firePatternIcon->setTexture(getFirePattern()->iconPath());
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

Player::AttributeSystem::AttributeSystem()
{
	baseAttributes = { 0.0f,0.0f, 0.0f, 0.0f, 0.0f };
	modifiers = { 0.0f,0.0f, 0.0f, 0.0f, 0.0f };
}

float Player::AttributeSystem::getAdjustedValue(Attribute id) const
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return 0.0f;
	}

	return baseAttributes.at(to_size_t(id)) + modifiers.at(to_size_t(id));
}

FlandrePC::FlandrePC(const ValueMap& args) :
	GObject(args),
	Player(args)
{}

void FlandrePC::setFirePatterns()
{
	firePatterns.push_back(make_unique<FlandreBigOrbPattern>(this));
	firePatterns.push_back(make_unique<FlandreFastOrbPattern>(this));
}

Player::AttributeSet FlandrePC::getAttributes() {
	return { 5, 500, 3, 9, 2.4f };
}

void FlandrePC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerBatMode")->second;
}

RumiaPC::RumiaPC(const ValueMap& args) :
	GObject(args),
	Player(args)
{}

void RumiaPC::setFirePatterns()
{
}

Player::AttributeSet RumiaPC::getAttributes() {
	return { 3, 900, 4.5, 12, 1.5f };
}

void RumiaPC::equipSpells() {
	//NO-OP
}


CirnoPC::CirnoPC(const ValueMap& args) :
	GObject(args),
	Player(args)
{}

void CirnoPC::setFirePatterns()
{
}

Player::AttributeSet CirnoPC::getAttributes() {
	return { 9, 300, 2, 6, 3.3f };
}

void CirnoPC::equipSpells() {
	//NO-OP
}
