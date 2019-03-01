//
//  Attributes.cpp
//  Koumachika
//
//  Created by Toni on 12/3/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "Attributes.hpp"
#include "macros.h"
#include "util.h"

const float AttributeSystem::maxElementDamage = 100.0f;
const float AttributeSystem::maxComboPoints = 75.0f;
const float AttributeSystem::comboPointsDrainPerSecond = 15.0f;

const map<Attribute, UpgradeInfo> AttributeSystem::upgradeAttributes = {
	{Attribute::maxHP, UpgradeInfo{ 1.0f, "sprites/hp_upgrade.png"}},
	{Attribute::maxMP, UpgradeInfo{ 1.0f, "sprites/mp_upgrade.png"}},
	{Attribute::agility, UpgradeInfo{ 1.0f, "sprites/agility_upgrade.png"}},
	{Attribute::attack, UpgradeInfo{ 0.25f, "sprites/attack_upgrade.png"}},
	{Attribute::shield, UpgradeInfo{ 1.0f, "sprites/shield_upgrade.png"}},
	{Attribute::ricochet, UpgradeInfo{1.0f, "sprites/ricochet_upgrade.png"}},
	{Attribute::bulletCount, UpgradeInfo{ 2.0f, "sprites/bullet_count_upgrade.png"}},
};

pair<float, float> AttributeSystem::calculateAgilityAttributes(float agility)
{
	if (agility == 0.0f) {
		return pair<float, float>(1.0f, 1.0f);
	}
	else {
		float speed = 2.0f * powf(1.33, agility - 1);
		return pair<float, float>(speed, speed*speed);
	}
}

AttributeSet AttributeSystem::getAttributeSet(const AttributeMap& input)
{
	AttributeSet result = getBlankAttributeSet();

	for (map<Attribute, float>::const_iterator it = input.cbegin(); it != input.cend(); ++it)
	{
		result[to_size_t(it->first)] = it->second;
	}

	if (result[to_size_t(Attribute::speed)] == 0.0f && result[to_size_t(Attribute::acceleration)] == 0.0f)
	{
		float agility = result[to_size_t(Attribute::agility)];
		pair<float, float> speedAccel = calculateAgilityAttributes(agility);

		result[to_size_t(Attribute::speed)] = speedAccel.first;
		result[to_size_t(Attribute::acceleration)] = speedAccel.second;
	}

	return result;
}

AttributeMap AttributeSystem::getAttributeMap(Attribute id, float val)
{
	return { {id,val} };
}

AttributeSet AttributeSystem::getBlankAttributeSet()
{
	AttributeSet result = getZeroArray<float, to_size_t(Attribute::end)>();

	result[to_size_t(Attribute::attack)] = 1.0f;
	result[to_size_t(Attribute::attackSpeed)] = 1.0f;
	result[to_size_t(Attribute::bulletCount)] = 1.0f;

	//Sensitivity multiplier should be 1.0 by default.
	result[to_size_t(Attribute::iceSensitivity)] = 1.0f;
	result[to_size_t(Attribute::sunSensitivity)] = 1.0f;
	result[to_size_t(Attribute::darknessSensitivity)] = 1.0f;
	result[to_size_t(Attribute::poisonSensitivity)] = 1.0f;
	result[to_size_t(Attribute::slimeSensitivity)] = 1.0f;

	return result;
}

AttributeSet AttributeSystem::getZeroAttributeSet()
{
	AttributeSet result = getZeroArray<float, to_size_t(Attribute::end)>();

	return result;
}

AttributeMap AttributeSystem::scale(const AttributeMap& input, float scale)
{
	AttributeMap result;

	for (auto entry : input) {
		result[entry.first] = entry.second * scale;
	}
	
	return result;
}

AttributeSystem::AttributeSystem() :
attributes(getBlankAttributeSet())
{}

AttributeSystem::AttributeSystem(const AttributeSet& baseAttributes) :
attributes(baseAttributes)
{}

AttributeSystem::AttributeSystem(const AttributeMap& baseAttributesMap) :
attributes(getAttributeSet(baseAttributesMap))
{}

float AttributeSystem::getAdjustedValue(Attribute id) const
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return 0.0f;
	}

	return attributes.at(to_size_t(id));
}

float AttributeSystem::getAttackMultiplier() const
{
	return attributes.at(to_size_t(Attribute::attack));
}

AttributeMap AttributeSystem::scaleBulletAttributes(const AttributeMap& bulletAttributes) const
{
	AttributeMap result = bulletAttributes;

	auto hp_it = result.find(Attribute::hp);
	if (hp_it != result.end()) {
		hp_it->second *= getAttackMultiplier();
	}

	return result;
}

float AttributeSystem::getWithinRange(float input, float min, float max)
{
	if (input < min)
		return min;
	else if (input > max)
		return max;
	else return input;
}

void AttributeSystem::setFullHP()
{
	attributes.at(to_size_t(Attribute::hp)) = attributes.at(to_size_t(Attribute::maxHP));
}

void AttributeSystem::setFullMP()
{
	attributes.at(to_size_t(Attribute::mp)) = attributes.at(to_size_t(Attribute::maxMP));
}

void AttributeSystem::setEmptyMP()
{
	attributes.at(to_size_t(Attribute::mp)) = 0;
}

void AttributeSystem::setStartMP()
{
	attributes.at(to_size_t(Attribute::mp)) = floor(attributes.at(to_size_t(Attribute::maxMP)) / 2.0);
}

void AttributeSystem::setFullPower()
{
	attributes.at(to_size_t(Attribute::power)) = attributes.at(to_size_t(Attribute::maxPower));
}

void AttributeSystem::setEmptyPower()
{
	attributes.at(to_size_t(Attribute::power)) = 0;
}

void AttributeSystem::setStartPower()
{
	attributes.at(to_size_t(Attribute::power)) = floor(attributes.at(to_size_t(Attribute::maxPower)) / 2.0);
}


void AttributeSystem::setHitProtection()
{
	attributes.at(to_size_t(Attribute::hitProtection)) = attributes.at(to_size_t(Attribute::hitProtectionInterval));
}

void AttributeSystem::setSpellCooldown()
{
	attributes.at(to_size_t(Attribute::spellCooldown)) = attributes.at(to_size_t(Attribute::spellCooldownInterval));
}

void AttributeSystem::resetCombo()
{
	attributes.at(to_size_t(Attribute::combo)) = 0.0f;
}

void AttributeSystem::setFullStamina()
{
	attributes.at(to_size_t(Attribute::stamina)) = attributes.at(to_size_t(Attribute::maxStamina));
}

void AttributeSystem::modifyAgility(float dx)
{
	attributes.at(to_size_t(Attribute::agility)) += dx;

	float agility = attributes.at(to_size_t(Attribute::agility));
	pair<float, float> speedAccel = calculateAgilityAttributes(agility);

	attributes.at(to_size_t(Attribute::speed)) = speedAccel.first;
	attributes.at(to_size_t(Attribute::acceleration)) = speedAccel.second;
}

void AttributeSystem::modifyIncidentAttribute(Attribute id, Attribute maxID, float x)
{
	attributes.at(to_size_t(id)) = getWithinRange(attributes.at(to_size_t(id)) + x, 0, attributes.at(to_size_t(maxID)));
}

void AttributeSystem::applyElementalDamage(Attribute id, Attribute sensitivity, float x)
{
	//If applying damage, the amount is scaled proportional to sensitivity
	if (x > 0) {
		attributes.at(to_size_t(id)) = getWithinRange(attributes.at(to_size_t(id)) + x * getAdjustedValue(sensitivity), 0, maxElementDamage);
	}
	else if (x < 0) {
		attributes.at(to_size_t(id)) = getWithinRange(attributes.at(to_size_t(id)) + x, 0, maxElementDamage);
	}
}

bool AttributeSystem::canApplyAttribute(Attribute id, float x)
{
	switch (id)
	{
	case Attribute::hp:
		return attributes.at(to_size_t(Attribute::hp)) + x <= attributes.at(to_size_t(Attribute::maxHP));
	case Attribute::mp:
		return attributes.at(to_size_t(Attribute::mp)) + x <= attributes.at(to_size_t(Attribute::maxMP));
	case Attribute::power:
		return attributes.at(to_size_t(Attribute::power)) + x <= attributes.at(to_size_t(Attribute::maxPower));
	default:
		log("AttributeSystem::canApplyAttribute: invalid attribute %d.", id);
		return true;
	}
}

void AttributeSystem::modifyAttribute(Attribute id, float x)
{
	switch (id)
	{
	case Attribute::hp:
		modifyIncidentAttribute(Attribute::hp, Attribute::maxHP, x);
		break;
	case Attribute::mp:
		modifyIncidentAttribute(Attribute::mp, Attribute::maxMP, x);
		break;
	case Attribute::power:
		modifyIncidentAttribute(Attribute::power, Attribute::maxPower, x);
		break;

	case Attribute::agility:
		modifyAgility(x);
		break;

	case Attribute::combo:
		attributes.at(to_size_t(Attribute::combo)) = getWithinRange(attributes.at(to_size_t(Attribute::combo)) + x, 0, maxComboPoints);
		break;

	case Attribute::iceDamage:
		applyElementalDamage(Attribute::iceDamage, Attribute::iceSensitivity, x);
		break;
	case Attribute::sunDamage:
		applyElementalDamage(Attribute::sunDamage, Attribute::sunSensitivity, x);
		break;
	case Attribute::darknessDamage:
		applyElementalDamage(Attribute::darknessDamage, Attribute::darknessSensitivity, x);
		break;
	case Attribute::poisonDamage:
		applyElementalDamage(Attribute::poisonDamage, Attribute::poisonSensitivity, x);
		break;
	case Attribute::slimeDamage:
		applyElementalDamage(Attribute::slimeDamage, Attribute::slimeSensitivity, x);
		break;
	default:
		attributes.at(to_size_t(id)) += x;
		break;
	}
}

void AttributeSystem::setAttribute(Attribute id, float x)
{
	attributes.at(to_size_t(id)) = x;
}

void AttributeSystem::timerDecrement(Attribute id)
{
	float& crnt_val = attributes.at(to_size_t(id));

	if (crnt_val != -1.0f) {
		crnt_val -= to_float(App::secondsPerFrame);
		crnt_val = max(crnt_val, 0.0f);
	}
}

bool AttributeSystem::isNonzero(Attribute id)
{
	return attributes.at(to_size_t(id)) != 0.0f;
}

void AttributeSystem::setProtection()
{
	attributes.at(to_size_t(Attribute::hitProtection)) = -1.0f;
}

void AttributeSystem::setTimedProtection(float seconds)
{
	attributes.at(to_size_t(Attribute::hitProtection)) = seconds;
}

void AttributeSystem::resetProtection()
{
	attributes.at(to_size_t(Attribute::hitProtection)) = 0.0f;
}
