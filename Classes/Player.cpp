//
//  Player.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "HUD.hpp"
#include "Bullet.hpp"
#include "AI.hpp"

#include "Player.hpp"

void Player::checkControls()
{
    auto kr = app->keyRegister;
    Vec2 moveDir = kr->getMoveKeyState();
    
    ai::applyDesiredVelocity(*this, toChipmunk(moveDir)*getSpeed(), accel);
    
    if(moveDir.lengthSquared() == 0)
         animSprite->reset();
    
    Vec2 facing = kr->getArrowKeyState();
    
    //Facing is not diagonal, horizontal direction will override.
    setDirection(toDirection(toChipmunk(facing)));
    
    //Player will automatically face their movement direction if look keys are not pressed
    if(facing.isZero() && body->getVel().lengthSq() > square(getSpeed())/2){
        setDirection(toDirection(toChipmunk(moveDir)));
    }
    
    //Fire if arrow key is pressed
    if(!facing.isZero())
    {
        fireIfPossible();
    }

    //Check item interaction
    if(kr->isKeyDown(Keys::enter))
    {
        GObject* item = getSensedObject();
        if(item)
        {
            InteractibleObject* interactible = dynamic_cast<InteractibleObject*>(item);

            if(interactible && interactible->canInteract())
                interactible->interact();
        }
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
    if(lastFireTime >= getFireInterval())
    {
        lastFireTime = 0;
        fire();
    }
}

void Player::fire()
{
    SpaceVect pos = body->getPos();
    pos += SpaceVect::ray(fireDist,body->getAngle());
    
    PlayerBaseBullet* bullet = new PlayerBaseBullet(body->getAngle(), pos);
    
    GScene::getSpace()->addObject(bullet);
}

void Player::hit(){
    if(hitProtectionCountdown <= 0){
        hitProtectionCountdown = hitProtectionTime;
        sprite->runAction(flickerAction(0.3, hitProtectionTime, 81));
    
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
