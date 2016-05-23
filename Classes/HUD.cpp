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
    for(int i=0;i<heartSprites.size(); ++i)
        removeChild(heartSprites.at(i));
    heartSprites.clear();
    
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

//const Color4F HUD::backgroundColor = Color4F(0,0,0,0.75);

const int HUD::fontSize = 32;

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
//    DrawNode* background = DrawNode::create();
//    addChild(background,1);
//    
//    background->drawSolidRect(
//        Vec2(0,App::height-height),
//        Vec2(App::width, App::height),
//        backgroundColor
//    );
    
    health = HealthBar::create();
    health->setPosition(32, App::height - height/2);
    addChild(health, 2);
    health->setMax(Player::defaultMaxHealth);
    
    objectiveCounter = new Counter("", 0);
    objectiveCounter->setPosition(Counter::spacing/2 + Counter::iconSize + 8, Counter::iconSize/2 + 8);
    addChild(objectiveCounter, 2);
    objectiveCounter->setVisible(false);
    
    interactionIcon = Sprite::create();
    interactionIcon->setPosition(App::width - 64, App::height - 64);
    interactionIcon->setScale(0.5);
    addChild(interactionIcon);
    
    return true;
}

void Counter::init(const string& iconRes, const int val)
{
    Node::init();
    
    icon = Sprite::create();
    counter = createTextLabel("", HUD::fontSize);
    
    //The center of the node will be the mid-point between the icon and the label.
    //This will avoid the visual distraction of moving the Counter node and thus the
    //icon if the width of the text label changes.
    addChild(icon);
    icon->setPosition(-(spacing+iconSize)/2, 0);
    
    //Label position will be set when its contents is set.
    addChild(counter);

    if(!iconRes.empty())
        setIcon(iconRes);
    setVal(val);
}

void Counter::setIcon(const string& iconRes)
{
    icon->setTexture(iconRes);
    float size = icon->getTexture()->getContentSizeInPixels().getBoundingDimension();
    if(size > iconSize)
        icon->setScale(iconSize / size);
}

void Counter::setVal(const int val)
{
    this->val = val;
    counter->setString(boost::lexical_cast<string>(val));
    
    float counterWidth = counter->getContentSize().width;
    counter->setPosition((spacing+counterWidth)/2, 0);
}