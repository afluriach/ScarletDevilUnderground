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
#include "GAnimation.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "Spell.hpp"

const int Player::defaultMaxHealth = 5;
const int Player::defaultMaxPower = 500;

const int Player::batModeInitialCost = 9;
const int Player::batModeCostPerSecond = 5;

const float Player::fireDist = 1.0f;

const float Player::interactCooldownTime = 0.1f;

const float Player::spellCooldownTime = 1.0f;
const float Player::hitProtectionTime = 2.4f;
const float Player::hitFlickerInterval = 0.3f;

const float Player::baseMaxSpeed = 3.0f;
const float Player::batModeMaxSpeed = 5.0f;

void Player::checkBatModeControls()
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

void Player::updateSpell()
{
    auto cr = app->control_register;
    
    if(isSpellActive())
    {
        if(cr->isControlActionPressed(ControlAction::spell1)){
            stop();
        }
    }
    else
    {
        spellCooldown = max(spellCooldown - App::secondsPerFrame, 0.0f);
        
        if(spellCooldown <= 0){
            if( cr->isControlActionPressed(ControlAction::spell1) &&
                power > Player::batModeInitialCost){
                    cast(make_unique<PlayerBatMode>(this, ValueMap()));
            }
        }
    }
}

void Player::stop()
{
    Spellcaster::stop();

    spellCooldown = spellCooldownTime;
    app->hud->power->runFlicker();
}

void Player::checkBaseControls()
{
    if(app->dialog)
        return;

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
    
    //Fire if arrow key is pressed
    if(!facing.isZero())
    {
        fireIfPossible();
    }

    //Check item interaction
    interactCooldown = max(interactCooldown - App::secondsPerFrame,0.0f);
    GObject* item = getSensedObject();
    InteractibleObject* interactible = dynamic_cast<InteractibleObject*>(item);

    if(interactible && interactible->canInteract())
    {
        app->hud->setInteractionIcon(interactible->interactionIcon());

        if(cr->isControlActionPressed(ControlAction::interact) && interactCooldown <= 0.0f){
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

void Player::updateFireTime()
{
    lastFireTime += App::secondsPerFrame;
}

void Player::update()
{
    if(!isSpellActive()){
        updateFireTime();
        updateHitTime();
        checkBaseControls();
    }
    updateSpell();
    
    if(health <= 0 && !app->suppressGameOver)
        app->playScene->triggerGameOver();
}


void Player::fireIfPossible()
{
    if(lastFireTime >= getFireInterval() && power > 0)
    {
        lastFireTime = 0;
        consumePower(1);
        fire();
    }
}

void Player::fire()
{
    SpaceVect pos = body->getPos();
    pos += SpaceVect::ray(fireDist,getAngle());
    
    GObject* bullet = new FlandreBigOrb1(getAngle(), pos);
    
    app->space->addObject(bullet);
}

void Player::hit(){
    if(hitProtectionCountdown <= 0 && !spellProtectionMode){
        hitProtectionCountdown = hitProtectionTime;
        sprite->runAction(flickerAction(hitFlickerInterval, hitProtectionTime, 81.0f));
        app->hud->health->runFlicker();
    
        health -= 1;
        if(health < 0) health = 0;
    }
}

void Player::setMaxHealth(int val){
    maxHealth = val;
    
    if(health > maxHealth){
        health = maxHealth;
        if(app->hud)
            app->hud->health->setValue(maxHealth);
    }
    
    if(app->hud)
        app->hud->health->setMax(maxHealth);
}

void Player::onCollectible(Collectible* coll)
{
    PowerUp* p = dynamic_cast<PowerUp*>(coll);
    
    if(p){
        power += 10;
        
        if(power > maxPower)
            power = maxPower;
        
        app->space->removeObject(coll);
    }
}
