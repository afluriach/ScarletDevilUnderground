//
//  Player.cpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "Bullet.hpp"

#include "Player.hpp"

void Player::setDirection(Direction d)
{
    animSprite->setDirection(d);
    body->setAngle(dirToPhysicsAngle(d));
}

void Player::checkControls()
{
    auto kr = app->keyRegister;
    
    Vec2 vel = kr->getMoveKeyState();
    body->setVel(toChipmunk(vel)*getSpeed());
    if(vel.lengthSquared() == 0)
         animSprite->reset();
    
    Vec2 facing = kr->getArrowKeyState();
    
    //Facing is not diagonal, horizontal direction will override.
    if(facing.y > 0)
        setDirection(Direction::upDir);
    else if(facing.y < 0)
        setDirection(Direction::downDir);
    if(facing.x < 0)
        setDirection(Direction::leftDir);
    else if(facing.x > 0)
        setDirection(Direction::rightDir);
    
    //Fire if arrow key is pressed
    if(!facing.isZero())
    {
        fireIfPossible();
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
