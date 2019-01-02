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

RadialMeter::RadialMeter(RadialMeterSettings settings) :
	RadialMeter(get<0>(settings), get<1>(settings), get<2>(settings))
{
}

RadialMeter::RadialMeter(string iconName, Color4F empty, Color4F filled) :
	iconName(iconName),
	filled(filled),
	empty(empty)
{
}

void RadialMeter::setValue(float v)
{
	float _v = v;

	if (_v < 0 || _v > 1.0) {
		_v = 1.0f;
	}

	if (_v != crntValue) {
		crntValue = _v;
		redraw();
	}
}

bool RadialMeter::init()
{
	Node::init();

	drawNode = DrawNode::create();
	addChild(drawNode, 1);

	icon = Sprite::create(iconName);
	addChild(icon, 2);

	setCascadeOpacityEnabled(true);

	redraw();

	return true;
}

void RadialMeter::setOpacity(GLubyte ch)
{
	Node::setOpacity(ch);
	redraw();
}

void RadialMeter::redraw()
{
	GLubyte alpha = _displayedOpacity;

	drawNode->clear();

	drawNode->drawSolidCircle(
		Vec2::ZERO,
		boundingSize - radiusMargin / 2, 
		0.0f,
		segments,
		opacityScale(Color4F::BLACK, alpha / 2)
	);

	drawNode->drawSolidCone(
		Vec2::ZERO,
		boundingSize - radiusMargin,
		0.0f,
		float_pi * 2.0 * crntValue,
		segments,
		opacityScale(filled, alpha)
	);

	drawNode->drawSolidCone(
		Vec2::ZERO,
		boundingSize - radiusMargin,
		float_pi * 2.0 * crntValue,
		float_pi * 2.0,
		segments,
		opacityScale(empty, alpha)
	);
}

const int MagicEffects::spacing = 128;

const float MagicEffects::totalAutohideTime = 1.5f;
const float MagicEffects::fadeoutTime = 0.75f;

const vector<pair<Attribute, RadialMeterSettings>> MagicEffects::meterSettings = {
	{Attribute::iceDamage,
		{"sprites/ui/snowflake.png",Color4F(0.2,0.33,0.7,0.5),Color4F(0.16, 0.2, 0.9, 1.0)}
	},
	{ Attribute::sunDamage,
		{ "sprites/ui/sun.png", Color4F(.3f,.3f,.12f,.5f), Color4F(.4f,.4f,.3f,1.0f) }
	},
	{ Attribute::poisonDamage,
		{ "sprites/ui/poison.png", Color4F(.11f,.52f,.74f,1.0f), Color4F(.29f,.11f,.62f,1.0f) }
	},
	{ Attribute::slimeDamage,
		{ "sprites/ui/slime.png", Color4F(.34f,.61f,.075f,1.0f), Color4F(.75f,.996f,.34f,1.0f) }
	},
	{ Attribute::hitProtection,
		{ "sprites/ui/hit_protection.png",Color4F(.42f,.29f,.29f,1.0f),Color4F(.86f,.16f,.19f,1.0f) }
	},
	{ Attribute::spellCooldown,
		{ "sprites/ui/spell_cooldown.png",Color4F(.4f,.4f,.4f,1.0f),Color4F(.37f,.56f,.57f,1.0f) }
	}
};

MagicEffects::MagicEffects()
{

}

bool MagicEffects::init()
{
	Node::init();

	for (auto entry : meterSettings)
	{
		RadialMeter* rm = Node::ccCreate<RadialMeter>(entry.second);
		rm->setScale(0.5f);
		rm->setVisible(false);
		addChild(rm);
		meters.insert(entry.first, rm);

		values.insert_or_assign(entry.first, 0);
		cooldownTimers.insert_or_assign(entry.first, 0.0f);
		fadeoutFlags.insert_or_assign(entry.first, false);
	}

	return true;
}

void MagicEffects::update()
{
	bool _reorganize = false;

	for (auto entry : meterSettings)
	{
		int val = values.find(entry.first)->second;
		float& timer = cooldownTimers.at(entry.first);

		if (val == 0 && timer != 0.0f) {
			timerDecrement(timer);

			if (timer < fadeoutTime && !fadeoutFlags.at(entry.first)) {
				meters.at(entry.first)->runAction(FadeOut::create(fadeoutTime));
				fadeoutFlags.insert_or_assign(entry.first, true);
			}

			if (timer == 0.0f) {
				_reorganize = true;
			}
		}
	}

	if (_reorganize) {
		reorganize();
	}
}

void MagicEffects::setPercentValue(Attribute element, int val)
{
	int prev = values.find(element)->second;
	RadialMeter* m = meters.find(element)->second;
	m->setValue(val / 100.0f);

	values.insert_or_assign(element, val);

	bool activated = prev == 0 && val != 0;
	bool deactivated = prev != 0 && val == 0;

	if (activated) {
		meters.at(element)->stopAllActions();
		meters.at(element)->setOpacity(255);
		fadeoutFlags.insert_or_assign(element, false);
	}

	if (deactivated) {
		cooldownTimers.insert_or_assign(element, totalAutohideTime);
	}

	if (activated || deactivated){
		reorganize();
	}
}

void MagicEffects::reorganize()
{
	int ypos = 0;

	for (auto entry : meterSettings)
	{
		RadialMeter* m = meters.at(entry.first);
		int val = values.find(entry.first)->second;
		float& timer = cooldownTimers.at(entry.first);
		bool visible = val != 0 || timer > 0.0f;

		m->setVisible(visible);

		if(visible) {
			m->setPosition(Vec2(0, ypos));
			ypos -= spacing;
		}

	}
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

	if (magicEffects) {
		magicEffects->update();
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
    
	magicEffects = Node::ccCreate<MagicEffects>();
	magicEffects->setPosition(App::width - 64 * scale, App::height - 64 * scale);
	addChild(magicEffects, 2);
	magicEffects->setScale(scale);

    objectiveCounter = Node::ccCreate<Counter>("", 0);
    objectiveCounter->setPosition(Counter::spacing/2 + Counter::iconSize + 8, Counter::iconSize/2 + 8);
    addChild(objectiveCounter, 2);
    objectiveCounter->setVisible(false);
	objectiveCounter->setScale(scale);
    
    interactionIcon = Sprite::create();
    interactionIcon->setPosition(App::width - 256*scale, App::height - 64*scale);
    interactionIcon->setScale(0.5*scale);
    addChild(interactionIcon);

	firePatternIcon = Sprite::create();
	firePatternIcon->setPosition(App::width - 384*scale, App::height - 64*scale);
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

void HUD::setMansionMode(bool val)
{
	isMansionMode = val;

	health->setVisible(!val);
	magic->setVisible(!val);
	power->setVisible(!val);

	firePatternIcon->setVisible(!val);
}

void HUD::setInteractionIcon(string val)
{
	interactionIcon->setVisible(!val.empty());
	interactionIcon->setTexture(val);
}

void HUD::setFirePatternIcon(string val)
{
	firePatternIcon->setVisible(!val.empty() && !isMansionMode);
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

void HUD::runMagicFlicker(float length, float interval)
{
	magic->runFlicker(length, interval);
}

void HUD::runPowerFlicker(float duration)
{
	power->runFlicker(duration);
}

void HUD::setPercentValue(Attribute element, int val) {
	magicEffects->setPercentValue(element, val);
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

void PowerMeter::runFlicker(float duration)
{
    icon->runAction(flickerTintAction(
        Player::hitFlickerInterval,
        duration,
        Color3B(127,127,127)
    ));
}
