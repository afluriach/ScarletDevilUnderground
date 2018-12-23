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

IconMeter::IconMeter(int iconSize, const string& filledIcon, const string& emptyIcon, int val, int maxVal) :
iconSize(iconSize),
filledIcon(filledIcon),
emptyIcon(emptyIcon),
crntVal(val),
maxVal(maxVal)
{
}

bool IconMeter::init()
{
	Node::init();

	setMax(maxVal);

	return true;
}

void IconMeter::setMax(int m)
{
    for_irange(i,0,iconSprites.size())
        removeChild(iconSprites.at(i));
    iconSprites.clear();
    
    for_irange(i,0,m)
    {
        Sprite* s = Sprite::create(emptyIcon);
        s->setPosition(32*i, 0);
        s->setScale(iconSize / s->getContentSize().width);
        iconSprites.pushBack(s);
        addChild(s);
    }

	maxVal = m;

	if (crntVal > maxVal)
		crntVal = maxVal;

	for (int i = 0; i<crntVal; ++i) {
		iconSprites.at(i)->setTexture(filledIcon);
	}
}

void IconMeter::setValue(int v)
{
    if(v == crntVal) return;
    
    if(v < 0) v = 0;

	if (v > maxVal) v = maxVal;

    for(int i=0;i<v; ++i){
        iconSprites.at(i)->setTexture(filledIcon);
    }
    for(int i=v; i<iconSprites.size(); ++i){
        iconSprites.at(i)->setTexture(emptyIcon);
    }
    crntVal = v;
}

void IconMeter::setElementalValue(float value)
{
	if (value == 0.0f) {
		setValue(0);
		setVisible(false);
	}
	else
	{
		setValue(floor(value));
		setVisible(true);
	}
}

void IconMeter::runFlicker(float duration, float interval)
{
    //TintTo action does not apply recursively.
	for_irange(i, 0, iconSprites.size()) {
		iconSprites.at(i)->runAction(
			flickerTintAction(
				interval,
				duration,
				Color3B(127, 127, 127)
			)
		);
	}
}

HealthBar::HealthBar() :
IconMeter(
	HealthBar::heartSize,
	"sprites/heart.png",
	"sprites/heart_empty.png",
	0,
	0
)
{
}

MagicBar::MagicBar() :
IconMeter(
	HealthBar::heartSize,
	"sprites/magic_card.png",
	"sprites/magic_card_empty.png",
	0,
	0
)
{
}

IceDamageBar::IceDamageBar() :
	IconMeter(
		HealthBar::heartSize,
		"sprites/ui/snowflake.png",
		"sprites/ui/snowflake_empty.png",
		0,
		4
	)
{
}

SunDamageBar::SunDamageBar() :
	IconMeter(
		HealthBar::heartSize,
		"sprites/ui/sun.png",
		"sprites/ui/sun_empty.png",
		0,
		4
	)
{
}


//const Color4F HUD::backgroundColor = Color4F(0,0,0,0.75);

const int HUD::fontSize = 32;

HUD::HUD(GSpace* space, object_ref<Player> player) :
player(player),
space(space)
{
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
		magic->setValue(p->getMagic());
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

	float scale = App::getScale();
    
    health = Node::ccCreate<HealthBar>();
    health->setPosition(32*scale, App::height - height/2);
    addChild(health, 2);
    health->setMax(1);
	health->setScale(scale);

	magic = Node::ccCreate<MagicBar>();
	magic->setPosition(scale*(32 + App::width / 4), App::height - height / 2);
	addChild(magic, 2);
	magic->setScale(scale);
	magic->setMax(5);
	magic->setValue(3);

    power = Node::ccCreate<PowerMeter>();
    power->setPosition(App::width/2, App::height - height/2);
    addChild(power,2);
    power->setVal(0);
	power->setScale(scale);
    
	iceDamage = Node::ccCreate<IceDamageBar>();
	iceDamage->setPosition(scale*(-64 + App::width * 3 / 4), App::height - height / 2);
	addChild(iceDamage, 2);
	iceDamage->setScale(scale);

	sunDamage = Node::ccCreate<SunDamageBar>();
	sunDamage->setPosition(scale*(64 + App::width * 3 / 4), App::height - height / 2);
	addChild(sunDamage, 2);
	sunDamage->setScale(scale);

    objectiveCounter = Node::ccCreate<Counter>("", 0);
    objectiveCounter->setPosition(Counter::spacing/2 + Counter::iconSize + 8, Counter::iconSize/2 + 8);
    addChild(objectiveCounter, 2);
    objectiveCounter->setVisible(false);
	objectiveCounter->setScale(scale);
    
    interactionIcon = Sprite::create();
    interactionIcon->setPosition(App::width - 64*scale, App::height - 64*scale);
    interactionIcon->setScale(0.5*scale);
    addChild(interactionIcon);

	firePatternIcon = Sprite::create();
	firePatternIcon->setPosition(App::width - 128*scale, App::height - 64*scale);
	firePatternIcon->setScale(0.5*scale);
	addChild(firePatternIcon);

    return true;
}

void HUD::setObjectiveCounter(string iconRes, int val)
{
	objectiveCounter->setVisible(true);
	objectiveCounter->setVal(val);
	objectiveCounter->setIcon(iconRes);
}

void HUD::setObjectiveCounterVisible(bool val)
{
	objectiveCounter->setVisible(val);
}

void HUD::showHealth(bool val)
{
	health->setVisible(val);
}

void HUD::setInteractionIcon(string val)
{
	interactionIcon->setVisible(!val.empty());
	interactionIcon->setTexture(val);
}

void HUD::setFirePatternIcon(string val)
{
	firePatternIcon->setVisible(!val.empty());
	firePatternIcon->setTexture(val);
}

void HUD::setHP(int v)
{
	health->setValue(v);
}

void HUD::setMaxHP(int v)
{
	health->setMax(v);
}

void HUD::setMP(int v)
{
	magic->setValue(v);
}

void HUD::setMaxMP(int v)
{
	magic->setMax(v);
}

void HUD::runHealthFlicker(float length, float interval)
{
	health->runFlicker(length, interval);
}

void HUD::runPowerFlicker()
{
	power->runFlicker();
}

void HUD::setIceDamage(float v)
{
	iceDamage->setElementalValue(v);
}

void HUD::setSunDamage(float v)
{
	sunDamage->setElementalValue(v);
}

Counter::Counter(const string& iconRes, const int val) :
val(val),
iconRes(iconRes)
{}

bool Counter::init()
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

	return true;
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
        boost::rational_cast<float>(Player::hitFlickerInterval),
        boost::rational_cast<float>(Player::spellCooldownTime),
        Color3B(127,127,127)
    ));
}
