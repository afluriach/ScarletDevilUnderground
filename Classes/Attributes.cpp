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

DamageInfo DamageInfo::operator*( float rhs)
{
	return DamageInfo{ mag * rhs, element, type };
}

const float AttributeSystem::maxElementDamage = 100.0f;
const float AttributeSystem::maxComboPoints = 75.0f;
const float AttributeSystem::comboPointsDrainPerSecond = 15.0f;

const unordered_map<Attribute, UpgradeInfo> AttributeSystem::upgradeAttributes = {
	{Attribute::maxHP, UpgradeInfo{ 25.0f, "sprites/hp_upgrade.png"}},
	{Attribute::maxMP, UpgradeInfo{ 25.0f, "sprites/mp_upgrade.png"}},
	{Attribute::maxStamina, UpgradeInfo{ 25.0f, "sprites/stamina_upgrade.png"}},
	{Attribute::agility, UpgradeInfo{ 1.0f, "sprites/agility_upgrade.png"}},
	{Attribute::attack, UpgradeInfo{ 0.25f, "sprites/attack_upgrade.png"}},
	{Attribute::attackSpeed, UpgradeInfo{ 0.25f, "sprites/attack_speed_upgrade.png"}},
	{Attribute::shieldLevel, UpgradeInfo{ 1.0f, "sprites/shield_upgrade.png"}},
	{Attribute::bulletSpeed, UpgradeInfo{ 0.5f, "sprites/bullet_speed_upgrade.png"}},
};

#define entry(x) (Attribute::x, #x)

const boost::bimap<Attribute, string> AttributeSystem::attributeNameMap = boost::assign::list_of<boost::bimap<Attribute, string>::relation>()
	entry(hp)
	entry(mp)
	entry(stamina)
	entry(hitProtection)
	entry(spellCooldown)

	entry(maxHP)
	entry(maxMP)
	entry(maxStamina)
	entry(hitProtectionInterval)
	entry(spellCooldownInterval)

	entry(hpRegen)
	entry(mpRegen)
	entry(staminaRegen)

	entry(keys)
	entry(combo)

	entry(touchDamage)

	entry(attack)
	entry(attackSpeed)
	entry(bulletSpeed)

	entry(shieldLevel)

	entry(stress)
	entry(stressDecay)
	entry(stressFromHits)
	entry(stressFromBlocks)
	entry(stressFromDetects)

	entry(agility)
	entry(speed)
	entry(acceleration)

	entry(iceSensitivity)
	entry(sunSensitivity)
	entry(darknessSensitivity)
	entry(poisonSensitivity)
	entry(slimeSensitivity)

	entry(iceDamage)
	entry(sunDamage)
	entry(darknessDamage)
	entry(poisonDamage)
	entry(slimeDamage)
;

#undef entry

float_pair AttributeSystem::calculateAgilityAttributes(float agility)
{
	if (agility == 0.0f) {
		return float_pair(1.0f, 1.0f);
	}
	else {
		float speed = 2.0f * powf(1.33, agility - 1);
		return float_pair(speed, speed*speed);
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
		float_pair speedAccel = calculateAgilityAttributes(agility);

		result[to_size_t(Attribute::speed)] = speedAccel.first;
		result[to_size_t(Attribute::acceleration)] = speedAccel.second;
	}

	return result;
}

AttributeMap AttributeSystem::getAttributeMap(Attribute id, float val)
{
	return { {id,val} };
}

AttributeMap AttributeSystem::getAttributeElementMap(Attribute element, float damage, float elementScale)
{
	return {
		{ Attribute::hp, -damage },
		{ element, damage * elementScale}
	};
}

AttributeSet AttributeSystem::getBlankAttributeSet()
{
	AttributeSet result = getZeroArray<float, to_size_t(Attribute::end)>();

	result[to_size_t(Attribute::attack)] = 1.0f;
	result[to_size_t(Attribute::attackSpeed)] = 1.0f;
	result[to_size_t(Attribute::bulletSpeed)] = 1.0f;

	//Sensitivity multiplier should be 1.0 by default.
	result[to_size_t(Attribute::bombSensitivity)] = 1.0f;
	result[to_size_t(Attribute::bulletSensitivity)] = 1.0f;
	result[to_size_t(Attribute::meleeSensitivity)] = 1.0f;

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

Attribute AttributeSystem::getElementSensitivity(Attribute element)
{
	return static_cast<Attribute>(
		to_int(element) - 
		to_int(Attribute::beginElementDamage) + 
		to_int(Attribute::beginElementSensitivity)
	);
}

Attribute AttributeSystem::getElement(Attribute elementSensitivity)
{
	return static_cast<Attribute>(
		to_int(elementSensitivity) -
		to_int(Attribute::beginElementSensitivity) +
		to_int(Attribute::beginElementDamage)
	);
}

AttributeMap AttributeSystem::scale(const AttributeMap& input, float scale)
{
	AttributeMap result;

	for (auto entry : input) {
		result[entry.first] = entry.second * scale;
	}
	
	return result;
}

AttributeMap AttributeSystem::add(const AttributeMap& a, const AttributeMap& b)
{
	AttributeMap result;

	for (pair<Attribute, float> entry : a) {
		if (result.find(entry.first) == result.end()) {
			result.insert_or_assign(entry.first, entry.second);
		}
		else {
			result.at(entry.first) += entry.second;
		}
	}

	for (pair<Attribute, float> entry : b) {
		if (result.find(entry.first) == result.end()) {
			result.insert_or_assign(entry.first, entry.second);
		}
		else {
			result.at(entry.first) += entry.second;
		}
	}
	
	return result;
}

float AttributeSystem::getAttribute(const AttributeMap& attr, Attribute id, float default)
{
	auto it = attr.find(id);
	return it != attr.end() ? it->second : default;
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

float AttributeSystem::operator[](Attribute id) const
{
	return attributes.at(to_size_t(id));
}

float AttributeSystem::get(string name) const
{
	auto it = attributeNameMap.right.find(name);
	if (it != attributeNameMap.right.end()) {
		return (*this)[it->second];
	}
	else {
		log("Attribute name %s not recognized.", name.c_str());
		return -1.0f;
	}
}

void AttributeSystem::set(string name, float val)
{
	auto it = attributeNameMap.right.find(name);
	if (it != attributeNameMap.right.end()) {
		set(it->second, val);
	}
}

void AttributeSystem::set(Attribute id, float x)
{
	attributes.at(to_size_t(id)) = x;
}

void AttributeSystem::_set(int id, float val)
{
	set(static_cast<Attribute>(id), val);
}

void AttributeSystem::update()
{
	applyIncidentRegen();
	applyElementDecay();
	timerDecrement(Attribute::stress, (*this)[Attribute::stressDecay]);
	if ((*this)[Attribute::hitProtection] != -1.0f)
		timerDecrement(Attribute::hitProtection);
}

void AttributeSystem::applyIncidentRegen()
{
	timerIncrement(Attribute::hp, Attribute::maxHP, (*this)[Attribute::hpRegen] * (*this)[Attribute::maxHP]);
	timerIncrement(Attribute::mp, Attribute::maxMP, (*this)[Attribute::mpRegen] * (*this)[Attribute::maxMP]);
	timerIncrement(Attribute::stamina, Attribute::maxStamina, (*this)[Attribute::staminaRegen] * (*this)[Attribute::maxStamina]);
}

void AttributeSystem::applyElementDecay()
{
	enum_foreach(Attribute, elem, beginElementDamage, endElementDamage) {
		timerDecrement(elem);
	}
}

float AttributeSystem::getHealthRatio() const
{
	return (*this)[Attribute::maxHP] > 0.0f ? (*this)[Attribute::hp] / (*this)[Attribute::maxHP] : 0.0f;
}

float AttributeSystem::getMagicRatio() const
{
	return (*this)[Attribute::maxMP] > 0.0f ? (*this)[Attribute::mp] / (*this)[Attribute::maxMP] : 0.0f;
}

float AttributeSystem::getStaminaRatio() const
{
	return (*this)[Attribute::maxStamina] > 0.0f ? (*this)[Attribute::stamina] / (*this)[Attribute::maxStamina] : 0.0f;
}

float AttributeSystem::applyDamage(DamageInfo damage)
{
	float elementSensitivity = damage.element != Attribute::end ? (*this)[getElementSensitivity(damage.element)] : 1.0f;
	float typeSensitivity = getTypeSensitivity(damage.type);

	modifyAttribute(Attribute::hp, -damage.mag * typeSensitivity * elementSensitivity);

	if (damage.element != Attribute::end) {
		modifyAttribute(damage.element, damage.mag * typeSensitivity);
	}

	return damage.mag * elementSensitivity * typeSensitivity;
}

void AttributeSystem::apply(const AttributeMap& effects)
{
	for (auto const& entry : effects)
	{
		modifyAttribute(entry.first, entry.second);
	}
}

float AttributeSystem::getAttackMultiplier() const
{
	return attributes.at(to_size_t(Attribute::attack));
}

float AttributeSystem::getTypeSensitivity(DamageType type) const
{
	switch (type)
	{
	case DamageType::bomb:
		return (*this)[Attribute::bombSensitivity];
	case DamageType::bullet:
		return (*this)[Attribute::bulletSensitivity];
	case DamageType::melee:
		return (*this)[Attribute::meleeSensitivity];
	default:
		return 1.0f;
	}
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

void AttributeSystem::setEmptyStamina()
{
	attributes.at(to_size_t(Attribute::stamina)) = 0.0f;
}

void AttributeSystem::modifyAgility(float dx)
{
	attributes.at(to_size_t(Attribute::agility)) += dx;

	float agility = attributes.at(to_size_t(Attribute::agility));
	float_pair speedAccel = calculateAgilityAttributes(agility);

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
		attributes.at(to_size_t(id)) = getWithinRange(attributes.at(to_size_t(id)) + x * (*this)[sensitivity], 0, maxElementDamage);
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
		return attributes.at(to_size_t(Attribute::hp)) < attributes.at(to_size_t(Attribute::maxHP));
	case Attribute::mp:
		return attributes.at(to_size_t(Attribute::mp)) < attributes.at(to_size_t(Attribute::maxMP));
	default:
		log("AttributeSystem::canApplyAttribute: invalid attribute %d.", to_int(id));
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
	case Attribute::stamina:
		modifyIncidentAttribute(Attribute::stamina, Attribute::maxStamina, x);
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

void AttributeSystem::modifyAttribute(Attribute mod, Attribute addend, float scale)
{
	attributes.at(to_size_t(mod)) += (*this)[addend] * scale;
}

void AttributeSystem::timerDecrement(Attribute id, float scale)
{
	float& crnt_val = attributes.at(to_size_t(id));

	if (crnt_val != -1.0f) {
		crnt_val -= to_float(App::secondsPerFrame * scale);
		crnt_val = max(crnt_val, 0.0f);
	}
}

void AttributeSystem::timerIncrement(Attribute id, Attribute maxID, float scale)
{
	float& crnt_val = attributes.at(to_size_t(id));
	float max_val = attributes.at(to_size_t(maxID));

	crnt_val += to_float(App::secondsPerFrame * scale);
	crnt_val = min(crnt_val, max_val);
}

bool AttributeSystem::isNonzero(Attribute id) const
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
