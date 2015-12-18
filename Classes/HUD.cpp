//
//  HUD.cpp
//  Koumachika
//
//  Created by Toni on 12/7/15.
//
//

#include "Prefix.h"
#include "HUD.hpp"

void HealthBar::setMax(int m)
{
    for(int i=0;i<m; ++i)
    {
        Sprite* s = Sprite::create("sprites/heart.png");
        s->setPosition(32*i, 0);
        s->setScale(heartSize / s->getContentSize().width);
        heartSprites.pushBack(s);
        addChild(s);
    }
}

void HealthBar::setValue(int v)
{
    if(v == crntVal) return;
    
    if(v < 0) v = 0;

    for(int i=0;i<v; ++i){
        heartSprites.at(i)->setTexture("sprites/heart.png");
    }
    for(int i=v; i<heartSprites.size(); ++i){
        heartSprites.at(i)->setTexture("sprites/heart_empty.png");
    }
    crntVal = v;
}

const Color4F HUD::backgroundColor = Color4F(0,0,0,0.75);

HUD::HUD() :
player(GScene::getSpace()->getObject<Player>("player"))
{
}
void HUD::update()
{
    health->setValue(player->getHealth());
}

bool HUD::init()
{
    Layer::init();
    
    //Create DrawNode in background layer
    DrawNode* background = DrawNode::create();
    addChild(background,1);
    
    background->drawSolidRect(
        Vec2(0,App::height-height),
        Vec2(App::width, App::height),
        backgroundColor
    );
    
    health = HealthBar::create();
    health->setPosition(32, App::height - height/2);
    addChild(health, 2);
    health->setMax(Player::maxHealth);
    
    return true;
}