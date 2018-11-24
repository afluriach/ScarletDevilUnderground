//
//  HUD.cpp
//  Koumachika
//
//  Created by Toni on 12/7/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "Graphics.h"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "macros.h"
#include "Player.hpp"
#include "scenes.h"
#include "util.h"

void HealthBar::setMax(int m)
{
    for_irange(i,0,heartSprites.size())
        removeChild(heartSprites.at(i));
    heartSprites.clear();
    
    for_irange(i,0,m)
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

void HealthBar::runFlicker(float duration)
{
    //TintTo action does not apply recursively.
    for_irange(i,0,heartSprites.size())
        heartSprites.at(i)->runAction(flickerTintAction(Player::hitFlickerInterval, duration,Color3B(127,127,127)));
}

//const Color4F HUD::backgroundColor = Color4F(0,0,0,0.75);

const int HUD::fontSize = 32;

HUD::HUD() :
player(app->space->getObject<Player>("player"))
{
    app->hud = this;
}

HUD::~HUD()
{
}

void HUD::update()
{
    if(player.isValid()){
        Player* p = player.get();
        health->setValue(p->getHealth());
        power->setVal(p->getPower());
    }
    else{
        setVisible(false);
    }
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
    health->setMax(1);
    
    power = PowerMeter::create();
    power->setPosition(App::width/2, App::height - height/2);
    addChild(power,2);
    power->setVal(0);
    
    objectiveCounter = new Counter("", 0);
    objectiveCounter->setPosition(Counter::spacing/2 + Counter::iconSize + 8, Counter::iconSize/2 + 8);
    addChild(objectiveCounter, 2);
    objectiveCounter->setVisible(false);
    
    interactionIcon = Sprite::create();
    interactionIcon->setPosition(App::width - 64, App::height - 64);
    interactionIcon->setScale(0.5);
    addChild(interactionIcon);

	firePatternIcon = Sprite::create();
	firePatternIcon->setPosition(App::width - 128, App::height - 64);
	firePatternIcon->setScale(0.5);
	addChild(firePatternIcon);

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
    if(val != this->val){
        this->val = val;
        counter->setString(boost::lexical_cast<string>(val));
        
        float counterWidth = counter->getContentSize().width;
        counter->setPosition((spacing+counterWidth)/2, 0);
    }
}


bool PowerMeter::init()
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

    icon->setTexture("sprites/power_up.png");
    setVal(0);
    
    return true;
}

void PowerMeter::setVal(int val)
{
    if(val != this->val){
        this->val = val;
        counter->setString(
            boost::str(boost::format("%.2f") % (val/100.0f) )
        );
        
        float counterWidth = counter->getContentSize().width;
        counter->setPosition((spacing+counterWidth)/2, 0);
    }
}

void PowerMeter::runFlicker()
{
    icon->runAction(flickerTintAction(
        Player::hitFlickerInterval,
        Player::spellCooldownTime,
        Color3B(127,127,127)
    ));
}
