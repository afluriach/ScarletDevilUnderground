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
	Agent(args),
	RegisterInit<Player>(this),
	RegisterUpdate<Player>(this)
{}

void Player::init()
{
	setFirePatterns();
	if(getFirePattern())
		app->hud->firePatternIcon->setTexture(getFirePattern()->iconPath());

	app->hud->health->setMax(health);

	equipSpells();
}

void Player::checkMovementControls(const ControlState& cs)
{
    SpaceVect moveDir = cs.left_v;
    
    ai::applyDesiredVelocity(*this, moveDir*getMaxSpeed(), getMaxAcceleration());
    
    if(moveDir.isZero())
         animSprite->reset();
    
    SpaceVect facing = cs.right_v;
    
    //Facing is not diagonal, horizontal direction will override.
    setDirection(toDirection(facing));
    
    //Player will automatically face their movement direction if look keys are not pressed
    if(facing.isZero() && body->getVel().lengthSq() > square(getMaxSpeed())/2){
        setDirection(toDirection(moveDir));
    }
}

void Player::updateSpellControls(const ControlState& cs)
{
    if(crntSpell.get())
    {
        if(cs.isControlActionPressed(ControlAction::spell1)){
            stopSpell();
        }
    }
    else
    {
        spellCooldown = max(spellCooldown - App::secondsPerFrame, 0.0f);
        
        if(spellCooldown <= 0){
            if( cs.isControlActionPressed(ControlAction::spell1) &&
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

void Player::checkFireControls(const ControlState& cs)
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

void Player::checkItemInteraction(const ControlState& cs)
{
	interactCooldown = max(interactCooldown - App::secondsPerFrame, 0.0f);
	GObject* item = getSensedObject();
	InteractibleObject* interactible = dynamic_cast<InteractibleObject*>(item);
	
	if(interactible && interactible->canInteract())
    {
        app->hud->setInteractionIcon(interactible->interactionIcon());

        if(cs.isControlActionPressed(ControlAction::interact) && interactCooldown <= 0.0f){
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

	ControlState cs = dynamic_cast<PlayScene*>(GScene::crntScene)->getControlData();

	checkMovementControls(cs);
	checkFireControls(cs);
	updateSpellControls(cs);
	checkItemInteraction(cs);

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

void Player::hit(int damage, shared_ptr<MagicEffect> effect){
    if(hitProtectionCountdown <= 0 && !spellProtectionMode){
        hitProtectionCountdown = attributeSystem.getAdjustedValue(Attribute::hitProtection);
        sprite->runAction(flickerAction(hitFlickerInterval, hitProtectionCountdown, 81.0f));
        app->hud->health->runFlicker(hitProtectionCountdown);

		Agent::hit(damage, effect);
    }
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

const AttributeMap FlandrePC::baseAttributes = boost::assign::map_list_of
	(Attribute::health, 5.0f)
	(Attribute::power, 500.0f)
	(Attribute::speed, 3.0f)
	(Attribute::acceleration, 9.0f)
	(Attribute::hitProtection, 2.4f)
	(Attribute::iceSensitivity, 2.0f)
;

FlandrePC::FlandrePC(const ValueMap& args) :
	GObject(args),
	Agent(args),
	Player(args)
{}

void FlandrePC::setFirePatterns()
{
	firePatterns.push_back(make_unique<FlandreBigOrbPattern>(this));
	firePatterns.push_back(make_unique<FlandreFastOrbPattern>(this));
}

void FlandrePC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerBatMode")->second;
}

const AttributeMap RumiaPC::baseAttributes = boost::assign::map_list_of
(Attribute::health, 3.0f)
(Attribute::power, 900.0f)
(Attribute::speed, 4.5f)
(Attribute::acceleration, 12.0f)
(Attribute::hitProtection, 1.5f)
(Attribute::iceSensitivity, 1.0f)
;

RumiaPC::RumiaPC(const ValueMap& args) :
	GObject(args),
	Agent(args),
	Player(args)
{}

void RumiaPC::setFirePatterns()
{
}

void RumiaPC::equipSpells() {
	equippedSpell = Spell::spellDescriptors.find("PlayerDarkMist")->second;
}

const AttributeMap CirnoPC::baseAttributes = boost::assign::map_list_of
(Attribute::health, 9.0f)
(Attribute::power, 300.0f)
(Attribute::speed, 2.0f)
(Attribute::acceleration, 6.0f)
(Attribute::hitProtection, 3.3f)
(Attribute::iceSensitivity, 0.0f)
;

CirnoPC::CirnoPC(const ValueMap& args) :
	GObject(args),
	Agent(args),
	Player(args)
{}

void CirnoPC::setFirePatterns()
{
	firePatterns.push_back(make_unique<CirnoLargeIceBulletPattern>(this));
}

void CirnoPC::equipSpells() {
	//NO-OP
}
