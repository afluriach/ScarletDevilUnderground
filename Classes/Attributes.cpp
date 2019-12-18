//
//  Attributes.cpp
//  Koumachika
//
//  Created by Toni on 12/3/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Attributes.hpp"

const float AttributeSystem::maxElementDamage = 100.0f;
const float AttributeSystem::maxComboPoints = 75.0f;
const float AttributeSystem::comboPointsDrainPerSecond = 15.0f;

const unordered_map<Attribute, UpgradeInfo> AttributeSystem::upgradeAttributes = {
	{Attribute::maxHP, UpgradeInfo{ 25.0f, "hp_upgrade"}},
	{Attribute::maxMP, UpgradeInfo{ 25.0f, "mp_upgrade"}},
	{Attribute::maxStamina, UpgradeInfo{ 25.0f, "stamina_upgrade"}},
	{Attribute::agility, UpgradeInfo{ 1.0f, "agility_upgrade"}},
	{Attribute::attack, UpgradeInfo{ 0.25f, "attack_upgrade"}},
	{Attribute::attackSpeed, UpgradeInfo{ 0.25f, "attack_speed_upgrade"}},
	{Attribute::shieldLevel, UpgradeInfo{ 1.0f, "shield_upgrade"}},
	{Attribute::bulletSpeed, UpgradeInfo{ 0.5f, "bullet_speed_upgrade"}},
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

	entry(currentSpeed)
	entry(speedRatio)
	entry(maxSpeed)
	entry(maxAcceleration)

	entry(bombSensitivity)
	entry(bulletSensitivity)
	entry(meleeSensitivity)

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

AttributeArray AttributeSystem::getAttributeSet(const AttributeMap& input)
{
	AttributeArray result = getBlankAttributeSet();

	for (map<Attribute, float>::const_iterator it = input.cbegin(); it != input.cend(); ++it)
	{
		result[to_size_t(it->first)] = it->second;
	}

	if (result[to_size_t(Attribute::maxSpeed)] == 0.0f && result[to_size_t(Attribute::maxAcceleration)] == 0.0f)
	{
		float agility = result[to_size_t(Attribute::agility)];
		float_pair speedAccel = calculateAgilityAttributes(agility);

		result[to_size_t(Attribute::maxSpeed)] = speedAccel.first;
		result[to_size_t(Attribute::maxAcceleration)] = speedAccel.second;
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

AttributeArray AttributeSystem::getBlankAttributeSet()
{
	AttributeArray result = getZeroArray<float, to_size_t(Attribute::end)>();

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

AttributeArray AttributeSystem::getZeroAttributeSet()
{
	AttributeArray result = getZeroArray<float, to_size_t(Attribute::end)>();

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

Attribute AttributeSystem::getAttribute(const string& name)
{
	auto it = attributeNameMap.right.find(name);
	return it != attributeNameMap.right.end() ? it->second : Attribute::end;
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

AttributeSystem::AttributeSystem(const AttributeArray& baseAttributes) :
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

void AttributeSystem::update(Agent* agent)
{
	applyIncidentRegen(hp);
	applyIncidentRegen(mp);
	applyIncidentRegen(stamina);

	applyElementDecay();
	timerDecrement(Attribute::stress, (*this)[Attribute::stressDecay]);
	if ((*this)[Attribute::hitProtection] != -1.0f)
		timerDecrement(Attribute::hitProtection);

	set(Attribute::currentSpeed, agent->getVel().length());
	set(Attribute::speedRatio,
		(*this)[Attribute::maxSpeed] > 0.0f ?
		(*this)[Attribute::currentSpeed] / (*this)[Attribute::maxSpeed] :
		0.0f
	);
}

void AttributeSystem::applyIncidentRegen(IncidentAttributeEntry entry)
{
	timerIncrement(entry.current, entry.maximum, (*this)[entry.regeneration] * (*this)[entry.maximum]);
}

void AttributeSystem::applyElementDecay()
{
	enum_foreach(Attribute, elem, beginElementDamage, endElementDamage) {
		timerDecrement(elem);
	}
}

float AttributeSystem::getIncidentRatio(IncidentAttributeEntry entry) const
{
	return (*this)[entry.maximum] > 0.0f ? (*this)[entry.current] / (*this)[entry.maximum] : 0.0f;
}

float AttributeSystem::getHealthRatio() const
{
	return getIncidentRatio(hp);
}

float AttributeSystem::getMagicRatio() const
{
	return getIncidentRatio(mp);
}

float AttributeSystem::getStaminaRatio() const
{
	return getIncidentRatio(stamina);
}

float AttributeSystem::applyDamage(DamageInfo damage)
{
	if ((*this)[Attribute::maxHP] <= 0.0f) {
		return -1.0f;
	}

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

float AttributeSystem::getWithinRange(float input, float min, float max)
{
	if (input < min)
		return min;
	else if (input > max)
		return max;
	else return input;
}

void AttributeSystem::setFull(IncidentAttributeEntry entry)
{
	attributes.at(to_size_t(entry.current)) = attributes.at(to_size_t(entry.maximum));
}

void AttributeSystem::setEmpty(IncidentAttributeEntry entry)
{
	attributes.at(to_size_t(entry.current)) = 0;
}

void AttributeSystem::setFullHP()
{
	setFull(hp);
}

void AttributeSystem::setFullMP()
{
	setFull(mp);
}

void AttributeSystem::setEmptyMP()
{
	setEmpty(mp);
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
	setFull(stamina);
}

void AttributeSystem::setEmptyStamina()
{
	setEmpty(stamina);
}

void AttributeSystem::modifyAgility(float dx)
{
	attributes.at(to_size_t(Attribute::agility)) += dx;

	float agility = attributes.at(to_size_t(Attribute::agility));
	float_pair speedAccel = calculateAgilityAttributes(agility);

	attributes.at(to_size_t(Attribute::maxSpeed)) = speedAccel.first;
	attributes.at(to_size_t(Attribute::maxAcceleration)) = speedAccel.second;
}

void AttributeSystem::modifyIncidentAttribute(IncidentAttributeEntry entry, float x)
{
	attributes.at(to_size_t(entry.current)) = getWithinRange(attributes.at(to_size_t(entry.current)) + x, 0, attributes.at(to_size_t(entry.maximum)));
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

bool AttributeSystem::canApplyIncidentAttribute(IncidentAttributeEntry entry) const
{
	return attributes.at(to_size_t(entry.current)) < attributes.at(to_size_t(entry.maximum));
}

bool AttributeSystem::canApplyAttribute(Attribute id, float x)
{
	switch (id)
	{
	case Attribute::hp:
		return canApplyIncidentAttribute(hp);
	case Attribute::mp:
		return canApplyIncidentAttribute(mp);
	case Attribute::stamina:
		return canApplyIncidentAttribute(stamina);

	case Attribute::keys:
		return true;
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
		modifyIncidentAttribute(hp, x);
		break;
	case Attribute::mp:
		modifyIncidentAttribute(mp, x);
		break;
	case Attribute::stamina:
		modifyIncidentAttribute(stamina, x);
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
		::timerDecrement(crnt_val, scale);
		crnt_val = max(crnt_val, 0.0f);
	}
}

void AttributeSystem::timerIncrement(Attribute id, Attribute maxID, float scale)
{
	float& crnt_val = attributes.at(to_size_t(id));
	float max_val = attributes.at(to_size_t(maxID));

	::timerIncrement(crnt_val, scale);
	crnt_val = min(crnt_val, max_val);
}

bool AttributeSystem::isZero(Attribute id) const
{
	return attributes.at(to_size_t(id)) == 0.0f;
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

bool AttributeSystem::hasHitProtection() const
{
	return isNonzero(Attribute::hitProtection);
}
