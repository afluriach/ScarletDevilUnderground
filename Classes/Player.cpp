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
#include "scenes.h"

void Player::checkControls()
{
    if(GScene::isDialogActive())
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
    GObject* item = getSensedObject();
    InteractibleObject* interactible = dynamic_cast<InteractibleObject*>(item);

    if(interactible && interactible->canInteract())
    {
        GScene::getHUD()->setInteractionIcon(interactible->interactionIcon());

        if(cr->isControlAction(ControlAction::interact)){
            interactible->interact();
        }
    }
    else
    {
        GScene::getHUD()->setInteractionIcon("");
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
    
    PlayerBaseBullet* bullet = new PlayerBaseBullet(getAngle(), pos);
    
    GScene::getSpace()->addObject(bullet);
}

void Player::hit(){
    if(hitProtectionCountdown <= 0){
        hitProtectionCountdown = hitProtectionTime;
        sprite->runAction(flickerAction(hitFlickerInterval, hitProtectionTime, 81.0f));
        GScene::getHUD()->health->runFlicker();
    
        health -= 1;
        if(health < 0) health = 0;
    }
}

void Player::setMaxHealth(int val){
    maxHealth = val;
    
    if(health > maxHealth){
        health = maxHealth;
        if(GScene::playScene())
            GScene::getHUD()->health->setValue(maxHealth);
    }
    
    if(GScene::playScene())
        GScene::getHUD()->health->setMax(maxHealth);
}

void Player::onCollectible(Collectible* coll)
{
    PowerUp* p = dynamic_cast<PowerUp*>(coll);
    
    if(p){
        power += 10;
        
        if(power > maxPower)
            power = maxPower;
        
        GScene::getSpace()->removeObject(coll);
    }
}
