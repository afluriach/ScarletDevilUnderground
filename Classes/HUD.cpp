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
#include "GScene.hpp"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "macros.h"
#include "Player.hpp"
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
		{ "sprites/ui/darkness.png", toColor4F(Color3B(80,20,136)), toColor4F(Color3B(53,13,89)) }
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
			floatToRoundedString(hp, 1.0f) % 
			floatToRoundedString(maxHP, 1.0f)
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

	timerDecrement(performanceStatsTimer);
	if (performanceStats && performanceStatsTimer <= 0.0f) {
		updatePerformanceStats();
		performanceStatsTimer = 1.0f;
	}

	for (auto& entry : autohideTimers) {
		timerDecrement(entry.second);

		if (entry.second <= 0.0f) {
			entry.first->setVisible(false);
		}
		else if (entry.second < 1.0f) {
			entry.first->setOpacity(entry.second * 255);
		}
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
	float meterEdgeOffset = (LinearMeter::boundingSize.x * 0.5f + LinearMeter::outlineWidth + 18.0f) * scale;
	float meterVerticalStep = (LinearMeter::boundingSize.y*1.0f + 2.0f*LinearMeter::outlineWidth + 9.0f) * scale;

	hpMeter = Node::ccCreate<LinearMeter>(LinearMeter::hpSettings);
    hpMeter->setPosition(meterEdgeOffset, App::height - meterVerticalStep);
    addChild(hpMeter, 2);
	hpMeter->setScale(scale);
	hpMeter->setVisible(false);

	mpMeter = Node::ccCreate<LinearMeter>(LinearMeter::mpSettings);
	mpMeter->setPosition(meterEdgeOffset, App::height - 2.0f*meterVerticalStep);
	addChild(mpMeter, 2);
	mpMeter->setScale(scale);
	mpMeter->setVisible(false);

	staminaMeter = Node::ccCreate<LinearMeter>(LinearMeter::staminaSettings);
	staminaMeter->setPosition(meterEdgeOffset, App::height - 3.0f*meterVerticalStep);
	addChild(staminaMeter, 2);
	staminaMeter->setScale(scale);
	staminaMeter->setMax(100);
	staminaMeter->setVisible(false);

	keyMeter = Node::ccCreate<Counter>("sprites/key.png", 0);
	keyMeter->setPosition(App::width * 0.4f, App::height - 64 * scale);
	addChild(keyMeter, 2);
	keyMeter->setScale(scale*0.8f);
	keyMeter->setVisible(false);

	mapFragmentMeter = Node::ccCreate<Counter>("sprites/map.png", 0);
	mapFragmentMeter->setPosition(App::width * 0.5f, App::height - 64 * scale);
	addChild(mapFragmentMeter, 2);
	mapFragmentMeter->setScale(scale * 0.8f);
	mapFragmentMeter->setVisible(false);

	magicEffects = Node::ccCreate<MagicEffects>();
	magicEffects->setPosition(App::width - 64 * scale, App::height - 64 * scale);
	addChild(magicEffects, 2);
	magicEffects->setScale(0.75f*scale);

    objectiveCounter = Node::ccCreate<Counter>(showAll ? "sprites/ui/glyph.png" : "", 0);
    objectiveCounter->setPosition(App::width * 0.3f, App::height - 64*scale);
    addChild(objectiveCounter, 2);
    objectiveCounter->setVisible(showAll);
	objectiveCounter->setScale(scale * 0.8f);
    
    interactionIcon = Sprite::create();
    interactionIcon->setPosition(App::width - 256*scale, App::height - 96*scale);
    interactionIcon->setScale(0.33*scale);
    addChild(interactionIcon);

	firePatternIcon = Sprite::create();
	firePatternIcon->setPosition(App::width - 480*scale, App::height - 128*scale);
	firePatternIcon->setScale(0.3f*scale);
	addChild(firePatternIcon);
	firePatternIcon->setVisible(false);

	spellIcon = Sprite::create();
	spellIcon->setPosition(App::width - 576 * scale, App::height - 96 * scale);
	spellIcon->setScale(scale * 0.5f);
	addChild(spellIcon);
	spellIcon->setVisible(false);

	powerAttackIcon = Sprite::create();
	powerAttackIcon->setPosition(App::width - 480 * scale, App::height - 64 * scale);
	powerAttackIcon->setScale(scale * 0.5f);
	addChild(powerAttackIcon);
	powerAttackIcon->setVisible(false);

	bombIcon = Sprite::create("sprites/scarlet_bomb.png");
	bombIcon->setPosition(App::width - 384 * scale, App::height - 96 * scale);
	bombIcon->setScale(scale * 0.5f);
	addChild(bombIcon);
	bombIcon->setVisible(false);

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
	resetVisibility(objectiveCounter);
	objectiveCounter->setVal(val);
	objectiveCounter->setIcon(iconRes);
}

void HUD::setObjectiveCounterVisible(bool val)
{
	objectiveCounter->setVisible(val);
}

void HUD::initMapCounter(int mapCount)
{
	mapFragmentMeter->setMaxVal(mapCount);
}

void HUD::setMapCounter(int val)
{
	resetVisibility(mapFragmentMeter);
	mapFragmentMeter->setVal(val);
}

void HUD::setPerformanceStats()
{
	if (!performanceStats && App::showTimers) {
		performanceStats = Label::createWithTTF("", "fonts/comfortaa.ttf", 18);
		performanceStats->setWidth(300);
		performanceStats->setPosition(App::width*0.1f, App::height*0.1f);
		addChild(performanceStats);
	}
}

void HUD::updatePerformanceStats()
{
	if (!App::showTimers) return;

	stringstream ss;
	App::timerMutex.lock();
	TimerTriplet object = App::timerSystem->getStats(TimerType::gobject);
	TimerTriplet physics = App::timerSystem->getStats(TimerType::physics);
	TimerTriplet render = App::timerSystem->getStats(TimerType::draw);
	App::timerMutex.unlock();

	ss << TimerSystem::timerStatString(object, "Object update") << "\n";
	ss << TimerSystem::timerStatString(physics, "Physics") << "\n";
	ss << TimerSystem::timerStatString(render, "Graphics");

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
	resetVisibility(firePatternIcon);
	firePatternIcon->setVisible(!val.empty() && !isMansionMode);
	firePatternIcon->setTexture(val);
}

void HUD::setSpellIcon(string val)
{
	resetVisibility(spellIcon);
	spellIcon->setVisible(!val.empty() && !isMansionMode);
	spellIcon->setTexture(val);
}

void HUD::setPowerAttackIcon(string val)
{
	resetVisibility(powerAttackIcon);
	powerAttackIcon->setVisible(!val.empty() && !isMansionMode);
	powerAttackIcon->setTexture(val);
}

void HUD::updateHUD(AttributeSystem playerAttributes)
{
	setMaxHP(to_int(playerAttributes[Attribute::maxHP]));
	setHP(to_int(playerAttributes[Attribute::hp]));
	setMaxMP(to_int(playerAttributes[Attribute::maxMP]));
	setMP(to_int(playerAttributes[Attribute::mp]));
	setMaxStamina(to_int(playerAttributes[Attribute::maxStamina]));
	setStamina(to_int(playerAttributes[Attribute::stamina]));

	setKeyCount(to_int(playerAttributes[Attribute::keys]));

	setEffect(Attribute::hitProtection, Attribute::hitProtectionInterval, &playerAttributes);
	setEffect(Attribute::spellCooldown, Attribute::spellCooldownInterval, &playerAttributes);
	setEffect(Attribute::combo, AttributeSystem::maxComboPoints, &playerAttributes);

	enum_foreach(Attribute, elem, beginElementDamage, endElementDamage) {
		setPercentValue(elem, to_int(playerAttributes[elem]));
	}
}

void HUD::setHP(int v)
{
	if (v != hpMeter->getValue()) {
		resetVisibility(hpMeter);
		hpMeter->setValue(v);
	}
}

void HUD::setMaxHP(int v)
{
	resetVisibility(hpMeter);
	hpMeter->setMax(v);
}

void HUD::setMP(int v)
{
	if (v != mpMeter->getValue()) {
		resetVisibility(mpMeter);
		mpMeter->setValue(v);
	}
}

void HUD::setMaxMP(int v)
{
	resetVisibility(mpMeter);
	mpMeter->setMax(v);
}

void HUD::setStamina(int v)
{
	if (v != staminaMeter->getValue()) {
		resetVisibility(staminaMeter);
		staminaMeter->setValue(v);
	}
}

void HUD::setMaxStamina(int v)
{
	resetVisibility(staminaMeter);
	staminaMeter->setMax(v);
}

void HUD::setKeyCount(int count)
{
	if (count != keyMeter->getVal()) {
		resetVisibility(keyMeter);
		keyMeter->setVal(count);
	}
}

void HUD::setEffect(Attribute id, Attribute max_id, const AttributeSystem* attr)
{
	setEffect(id, (*attr)[max_id], attr);
}

void HUD::setEffect(Attribute id, float maxVal, const AttributeSystem* attr)
{
	float val = (*attr)[id];
	int percent = (val >= 0.0f  && maxVal > 0.0f ? val / maxVal * 100.0f : 100);
	setPercentValue(id, percent);
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

void HUD::showHidden()
{
	showHiddenNode(hpMeter);
	showHiddenNode(mpMeter);
	showHiddenNode(staminaMeter);
	showHiddenNode(keyMeter);
	showHiddenNode(mapFragmentMeter);
	showHiddenNode(firePatternIcon);
	showHiddenNode(spellIcon);
	showHiddenNode(powerAttackIcon);
	showHiddenNode(bombIcon);
}

void HUD::resetAutohide()
{
	resetNodeAutohide(hpMeter);
	resetNodeAutohide(mpMeter);
	resetNodeAutohide(staminaMeter);
	resetNodeAutohide(keyMeter);
	resetNodeAutohide(mapFragmentMeter);
	resetNodeAutohide(firePatternIcon);
	resetNodeAutohide(spellIcon);
	resetNodeAutohide(powerAttackIcon);
	resetNodeAutohide(bombIcon);
}

void HUD::showHiddenNode(Node* node)
{
	node->setVisible(true);
	node->setOpacity(255);
}

void HUD::resetNodeAutohide(Node* node)
{
	auto it = autohideTimers.find(node);
	node->setVisible(it != autohideTimers.end());
	if (it != autohideTimers.end())
		node->setOpacity(it->second < 1.0f ? it->second * 255 : 255);
}

void HUD::resetVisibility(Node* node)
{
	node->setVisible(true);
	node->setOpacity(255);
	autohideTimers.insert_or_assign(node, 2.0f);
}

Counter::Counter(const string& iconRes, int val, int maxVal) :
val(val),
maxVal(maxVal),
iconRes(iconRes)
{}

bool Counter::init()
{
    Node::init();
    
    icon = Sprite::create();
    counter = createTextLabel(makeLabel(), HUD::fontSize);
	repositionText();

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

void Counter::setVal(int _val)
{
    if(_val != val){
        val = _val;
        counter->setString(makeLabel());
		repositionText();
    }
}

void Counter::setMaxVal(int _maxVal)
{
	if (maxVal != _maxVal) {
		maxVal = _maxVal;
		counter->setString(makeLabel());
		repositionText();
	}
}

string Counter::makeLabel()
{
	if (maxVal == 0)
		return boost::lexical_cast<string>(val);
	else
		return boost::str(boost::format("%s / %s") %
			boost::lexical_cast<string>(val) %
			boost::lexical_cast<string>(maxVal)
		);
}

void Counter::repositionText()
{
	float counterWidth = counter->getContentSize().width;
	counter->setPosition((spacing + counterWidth) / 2, 0);
}
