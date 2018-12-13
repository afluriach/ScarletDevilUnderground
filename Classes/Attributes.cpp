//
//  Attributes.cpp
//  Koumachika
//
//  Created by Toni on 12/3/18.
//
//

#include "Prefix.h"

#include "Attributes.hpp"
#include "util.h"

const float maxElementDamage = 100.0f;

AttributeSet getAttributeSet(const AttributeMap& input)
{
	AttributeSet result = getBlankAttributeSet();

	for (map<Attribute, float>::const_iterator it = input.cbegin(); it != input.cend(); ++it)
	{
		result[static_cast<size_t>(it->first)] = it->second;
	}

	return result;
}

AttributeMap getAttributeMap(Attribute id, float val)
{
	return { {id,val} };
}

AttributeSet getBlankAttributeSet()
{
	AttributeSet result = getZeroArray<float, to_size_t(Attribute::end)>();

	//Sensitivity multiplier should be 1.0 by default.
	result[to_size_t(Attribute::iceSensitivity)] = 1.0f;

	return result;
}

AttributeSet getZeroAttributeSet()
{
	AttributeSet result = getZeroArray<float, to_size_t(Attribute::end)>();

	return result;
}


AttributeSystem::AttributeSystem() :
baseAttributes(getBlankAttributeSet()),
modifiers(getZeroAttributeSet())
{}

AttributeSystem::AttributeSystem(const AttributeSet& baseAttributes) :
baseAttributes(baseAttributes),
modifiers(getZeroAttributeSet())
{}

AttributeSystem::AttributeSystem(const AttributeMap& baseAttributesMap) :
baseAttributes(getAttributeSet(baseAttributesMap)),
modifiers(getZeroAttributeSet())
{}

float AttributeSystem::getAdjustedValue(Attribute id) const
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return 0.0f;
	}

	return baseAttributes.at(to_size_t(id)) + modifiers.at(to_size_t(id));
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
	baseAttributes.at(to_size_t(Attribute::hp)) = baseAttributes.at(to_size_t(Attribute::maxHP));
}

void AttributeSystem::setFullMP()
{
	baseAttributes.at(to_size_t(Attribute::mp)) = baseAttributes.at(to_size_t(Attribute::maxMP));
}

void AttributeSystem::setEmptyMP()
{
	baseAttributes.at(to_size_t(Attribute::mp)) = 0;
}

void AttributeSystem::setFullPower()
{
	baseAttributes.at(to_size_t(Attribute::power)) = baseAttributes.at(to_size_t(Attribute::maxPower));
}

void AttributeSystem::setEmptyPower()
{
	baseAttributes.at(to_size_t(Attribute::power)) = 0;
}

void AttributeSystem::modifyAttribute(Attribute id, float x)
{
	switch (id)
	{
	case Attribute::hp:
		baseAttributes.at(to_size_t(id)) = getWithinRange(baseAttributes.at(to_size_t(id)) + x, 0, baseAttributes.at(to_size_t(Attribute::maxHP)));
		break;
	case Attribute::mp:
		baseAttributes.at(to_size_t(id)) = getWithinRange(baseAttributes.at(to_size_t(id)) + x, 0, baseAttributes.at(to_size_t(Attribute::maxMP)));
		break;
	case Attribute::power:
		baseAttributes.at(to_size_t(id)) = getWithinRange(baseAttributes.at(to_size_t(id)) + x, 0, baseAttributes.at(to_size_t(Attribute::maxPower)));
		break;

	case Attribute::iceDamage:
		baseAttributes.at(to_size_t(id)) = getWithinRange(baseAttributes.at(to_size_t(id)) + x*getAdjustedValue(Attribute::iceSensitivity), 0, maxElementDamage);
		break;

	default:
		modifiers.at(to_size_t(id)) += x;
		break;
	}
}
