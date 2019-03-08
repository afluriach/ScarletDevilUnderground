//
//  HUD.cpp
//  Koumachika
//
//  Created by Toni on 12/7/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "Enemy.hpp"
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
			flickerTint(
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

	cone = Node::ccCreate<RadialMeterShader>(
		filled,
		empty, 
		(boundingSize - radiusMargin) / 2,
		Vec2::ZERO,
		0.0f
	);
	cone->setContentSize(CCSize(boundingSize - radiusMargin, boundingSize - radiusMargin));
	addChild(cone, 1);

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

	cone->setAngle(float_pi*2.0 * crntValue);
	cone->setColors(opacityScale(filled, alpha), opacityScale(empty, alpha));
}

const Vec2 LinearMeter::boundingSize = Vec2(192,48);
const float LinearMeter::meterOffset = 32.0f;
const float LinearMeter::outlineWidth = 8;

LinearMeter::LinearMeter(LinearMeterSettings settings) :
	settings(settings)
{
}

bool LinearMeter::init()
{
	Node::init();

	draw = DrawNode::create();
	addChild(draw);

	label = Label::createWithTTF(
		" 0 / 0",
		"fonts/comfortaa.ttf",
		boundingSize.y / 2,
		CCSize(boundingSize),
		TextHAlignment::CENTER,
		TextVAlignment::CENTER
	);

	addChild(label);

	return true;
}

void LinearMeter::setValue(float newValue)
{
	crntValue = newValue;
	redraw();
}

void LinearMeter::setMax(float maxValue)
{
	this->maxValue = maxValue;
	redraw();
}

void LinearMeter::redraw()
{
	if (maxValue <= 0.0f) return;

	float ratio = crntValue / maxValue;

	draw->clear();

	draw->drawSolidRect(
		Vec2(-0.5f * boundingSize.x - outlineWidth, meterOffset - outlineWidth),
		Vec2(boundingSize.x*0.5f + outlineWidth, boundingSize.y + outlineWidth),
		Color4F::BLACK
	);

	draw->drawSolidRect(
		Vec2(-0.5f * boundingSize.x, meterOffset),
		Vec2(boundingSize.x*(ratio-0.5f),boundingSize.y),
		settings.fillColor
	);
	draw->drawSolidRect(
		Vec2(boundingSize.x*(ratio - 0.5f), meterOffset),
		Vec2(boundingSize.x * 0.5f, boundingSize.y),
		settings.emptyColor
	);

	label->setString(boost::str(
		boost::format("%s / %s") % 
		boost::lexical_cast<string>(crntValue) %
		boost::lexical_cast<string>(maxValue)
	));
}

const int MagicEffects::spacing = 128;

const float MagicEffects::totalAutohideTime = 1.5f;
const float MagicEffects::fadeoutTime = 0.75f;

const vector<pair<Attribute, RadialMeterSettings>> MagicEffects::meterSettings = {
	{Attribute::iceDamage,
		{"sprites/ui/snowflake.png",Color4F(0.2f,0.33f,0.7f,0.5f),Color4F(0.16f, 0.2f, 0.9f, 1.0f)}
	},
	{ Attribute::sunDamage,
		{ "sprites/ui/sun.png", Color4F(.3f,.3f,.12f,.5f), Color4F(.4f,.4f,.3f,1.0f) }
	},
	{ Attribute::darknessDamage,
		{ "sprites/ui/darkness.png", toColor4F(Color3B(13,13,89)), toColor4F(Color3B(53,13,89)) }
	},
	{ Attribute::poisonDamage,
		{ "sprites/ui/poison.png", Color4F(.11f,.52f,.74f,1.0f), Color4F(.29f,.11f,.62f,1.0f) }
	},
	{ Attribute::slimeDamage,
		{ "sprites/ui/slime.png", Color4F(.34f,.61f,.075f,1.0f), Color4F(.75f,.996f,.34f,1.0f) }
	},
	{ Attribute::combo,
		{ "sprites/power1.png",Color4F(.42f,.29f,.29f,1.0f),Color4F(.86f,.16f,.19f,1.0f) }
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
		rm->setVisible(HUD::showAll);
		addChild(rm);
		meters.insert(entry.first, rm);

		values.insert_or_assign(entry.first, 0);
		cooldownTimers.insert_or_assign(entry.first, 0.0f);
		fadeoutFlags.insert_or_assign(entry.first, false);
	}

	if (HUD::showAll) reorganize();

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

		m->setVisible(visible || HUD::showAll);

		if(visible || HUD::showAll) {
			m->setPosition(Vec2(0, ypos));
			ypos -= spacing;
		}
	}
}

const int EnemyInfo::hWidth = 128;
const int EnemyInfo::hHeight = 24;

EnemyInfo::EnemyInfo()
{}

bool EnemyInfo::init()
{
	Node::init();

	name = createTextLabel("", 24);
	name->setPosition(Vec2(0, 24));
	addChild(name, 2);

	hpLabel = createTextLabel("0 / 0", 18);
	hpLabel->setPosition(Vec2(0, 0));
	addChild(hpLabel, 2);

	healthBar = DrawNode::create();
	healthBar->setPosition(Vec2(0, 16));
	addChild(healthBar, 1);

	return true;
}

void EnemyInfo::update()
{
	healthBar->clear();

	if (maxHP > 0.0f) {
		float hpRatio = hp / maxHP;
		healthBar->drawSolidRect(Vec2(-hWidth, -hHeight), Vec2(hWidth, hHeight), Color4F(0.0f, 0.0f, 0.0f, 1.0f));
		healthBar->drawSolidRect(Vec2(-hWidth, -hHeight), Vec2(-hWidth + 2.0f*hWidth * hpRatio, hHeight), Color4F(1.0f, 0.0f, 0.0f, 1.0f));
		hpLabel->setString(boost::str(
			boost::format("%s / %s") %
			boost::lexical_cast<string>(hp) % 
			boost::lexical_cast<string>(maxHP)
		));
	}
}

void EnemyInfo::setEnemy(string _name, float _hp, float _maxHP)
{
	hp = _hp;
	maxHP = _maxHP;

	name->setString(_name);
}

void EnemyInfo::setEnemyHealth(float _hp)
{
	hp = _hp;
}

bool EnemyInfo::isValid()
{
	return hp > 0.0f;
}

//const Color4F HUD::backgroundColor = Color4F(0,0,0,0.75);

const LinearMeterSettings HUD::hpSettings = LinearMeterSettings{
	Color4F(.86f,.16f,.19f,1.0f),
	Color4F(.42f,.29f,.29f,1.0f),
//	"sprites/hp_upgrade.png"
};

const LinearMeterSettings HUD::mpSettings = LinearMeterSettings{
	Color4F(.37f,.56f,.57f,1.0f),
	Color4F(.4f,.4f,.4f,1.0f),
//	"sprites/mp_upgrade.png"
};

const LinearMeterSettings HUD::staminaSettings = LinearMeterSettings{
	Color4F(.47f,.75f,.18f,1.0f),
	Color4F(.44f,.51f,.36f,1.0f),
	//	"sprites/mp_upgrade.png"
};

const int HUD::fontSize = 32;

HUD::HUD(GSpace* space) :
space(space)
{
}

HUD::~HUD()
{
}

void HUD::update()
{
	if (magicEffects) {
		magicEffects->update();
	}

	enemyInfo->setVisible(enemyInfo->isValid());
	enemyInfo->update();
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
    
	hpMeter = Node::ccCreate<LinearMeter>(hpSettings);
    hpMeter->setPosition(App::width * 0.167f, App::height - LinearMeter::boundingSize.y*2.0f*scale);
    addChild(hpMeter, 2);
	hpMeter->setScale(scale);

	mpMeter = Node::ccCreate<LinearMeter>(mpSettings);
	mpMeter->setPosition(App::width*0.333f, App::height - LinearMeter::boundingSize.y*2.0f*scale);
	addChild(mpMeter, 2);
	mpMeter->setScale(scale);

	staminaMeter = Node::ccCreate<LinearMeter>(staminaSettings);
	staminaMeter->setPosition(App::width*0.5f, App::height - LinearMeter::boundingSize.y*2.0f*scale);
	addChild(staminaMeter, 2);
	staminaMeter->setScale(scale);
	staminaMeter->setMax(100);

	keyMeter = Node::ccCreate<Counter>("sprites/key.png", 0);
	keyMeter->setPosition(App::width * 0.7f, App::height - 64 * scale);
	addChild(keyMeter, 2);
	keyMeter->setScale(scale*0.8f);

	magicEffects = Node::ccCreate<MagicEffects>();
	magicEffects->setPosition(App::width - 64 * scale, App::height - 64 * scale);
	addChild(magicEffects, 2);
	magicEffects->setScale(0.75f*scale);

    objectiveCounter = Node::ccCreate<Counter>(showAll ? "sprites/ui/glyph.png" : "", 0);
    objectiveCounter->setPosition(App::width * 0.7f, App::height - 128*scale);
    addChild(objectiveCounter, 2);
    objectiveCounter->setVisible(showAll);
	objectiveCounter->setScale(scale * 0.8f);
    
    interactionIcon = Sprite::create();
    interactionIcon->setPosition(App::width - 256*scale, App::height - 64*scale);
    interactionIcon->setScale(0.33*scale);
    addChild(interactionIcon);

	firePatternIcon = Sprite::create();
	firePatternIcon->setPosition(App::width - 384*scale, App::height - 64*scale);
	firePatternIcon->setScale(0.3f*scale);
	addChild(firePatternIcon);

	enemyInfo = Node::ccCreate<EnemyInfo>();
	enemyInfo->setPosition(App::width - (EnemyInfo::hWidth+24)*scale, 24*scale);
	enemyInfo->setScale(scale);
	addChild(enemyInfo);

	if (showAll){
		setEnemyInfo("Enemy III", 50.0f, 100.0f);
	}

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

void HUD::setPerformanceStats(TimerTriplet objects, TimerTriplet physics)
{
	if (!performanceStats) {
		performanceStats = Label::createWithTTF("", "fonts/comfortaa.ttf", 18);
		performanceStats->setWidth(300);
		performanceStats->setPosition(160, 90);
		addChild(performanceStats);
	}

	stringstream ss;

	ss << TimerSystem::timerStatString(objects, "Object update") << "\n";
	ss << TimerSystem::timerStatString(physics, "Physics") << "\n";

	performanceStats->setString(ss.str());
}

void HUD::clearPerformanceStats()
{
	if (performanceStats) {
		removeChild(performanceStats);
		performanceStats = nullptr;
	}
}


void HUD::setMansionMode(bool val)
{
	isMansionMode = val;

	hpMeter->setVisible(!val);
	mpMeter->setVisible(!val);
	staminaMeter->setVisible(!val);
	keyMeter->setVisible(!val);

	firePatternIcon->setVisible(!val);
}

void HUD::setInteractionIcon(string val)
{
	interactionIcon->setVisible(!val.empty() || showAll);
	interactionIcon->setTexture(showAll && val.empty() ? "sprites/ui/dialog.png" : val);
}

void HUD::setFirePatternIcon(string val)
{
	firePatternIcon->setVisible(!val.empty() && !isMansionMode);
	firePatternIcon->setTexture(val);
}

void HUD::setHP(int v)
{
	hpMeter->setValue(v);
}

void HUD::setMaxHP(int v)
{
	hpMeter->setMax(v);
}

void HUD::setMP(int v)
{
	mpMeter->setValue(v);
}

void HUD::setMaxMP(int v)
{
	mpMeter->setMax(v);
}

void HUD::setStamina(int v)
{
	staminaMeter->setValue(v);
}

void HUD::setMaxStamina(int v)
{
	staminaMeter->setMax(v);
}

void HUD::setKeyCount(int count)
{
	keyMeter->setVal(count);
}

void HUD::runHealthFlicker(float length, float interval)
{
//	health->runFlicker(length, interval);
}

void HUD::runMagicFlicker(float length, float interval)
{
//	magic->runFlicker(length, interval);
}

void HUD::setPercentValue(Attribute element, int val) {
	magicEffects->setPercentValue(element, val);
}

void HUD::setEnemyInfo(string name, float hp, float maxHP)
{
	enemyInfo->setVisible(true);
	enemyInfo->setEnemy(name, hp, maxHP);
}

void HUD::updateEnemyInfo(float hp)
{
	enemyInfo->setEnemyHealth(hp);
}

void HUD::clearEnemyInfo()
{
	enemyInfo->setVisible(false);
}

Counter::Counter(const string& iconRes, const int val) :
val(val),
iconRes(iconRes)
{}

bool Counter::init()
{
    Node::init();
    
    icon = Sprite::create();
    counter = createTextLabel(boost::lexical_cast<string>(val), HUD::fontSize);
	float counterWidth = counter->getContentSize().width;
	counter->setPosition((spacing + counterWidth) / 2, 0);

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
