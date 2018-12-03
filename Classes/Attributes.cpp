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

AttributeSet getAttributeSet(const AttributeMap& input)
{
	AttributeSet result = getBlankAttributeSet();

	for (map<Attribute, float>::const_iterator it = input.cbegin(); it != input.cend(); ++it)
	{
		result[static_cast<size_t>(it->first)] = it->second;
	}

	return result;
}

AttributeSet getBlankAttributeSet()
{
	return getZeroArray<float, to_size_t(Attribute::end)>();
}

AttributeSystem::AttributeSystem() :
baseAttributes(getBlankAttributeSet()),
modifiers(getBlankAttributeSet())
{}

AttributeSystem::AttributeSystem(const AttributeSet& baseAttributes) :
baseAttributes(baseAttributes),
modifiers(getBlankAttributeSet())
{}

AttributeSystem::AttributeSystem(const AttributeMap& baseAttributesMap) :
baseAttributes(getAttributeSet(baseAttributesMap)),
modifiers(getBlankAttributeSet())
{}

float AttributeSystem::getAdjustedValue(Attribute id) const
{
	if (id >= Attribute::end) {
		log("invalid attribute %d", id);
		return 0.0f;
	}

	return baseAttributes.at(to_size_t(id)) + modifiers.at(to_size_t(id));
}
