//
//  Player.cpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#include "Player.hpp"

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
        animSprite->setDirection(PatchConAnimation::Direction::up);
    else if(facing.y < 0)
        animSprite->setDirection(PatchConAnimation::Direction::down);
    if(facing.x < 0)
        animSprite->setDirection(PatchConAnimation::Direction::left);
    else if(facing.x > 0)
        animSprite->setDirection(PatchConAnimation::Direction::right);

}