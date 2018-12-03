//
//  Attributes.hpp
//  Koumachika
//
//  Created by Toni on 12/3/18.
//
//

#ifndef Attributes_hpp
#define Attributes_hpp

#include "enum.h"

enum class Attribute {
	health = 0,
	power,
	speed,
	acceleration,
	hitProtection,
	iceSensitivity,

	end,
};

typedef array<float, to_size_t(Attribute::end)> AttributeSet;
typedef map<Attribute, float> AttributeMap;

 AttributeSet getAttributeSet(const AttributeMap& input);
 AttributeSet getBlankAttributeSet();

class AttributeSystem
{
public:
	AttributeSet baseAttributes;
	AttributeSet modifiers;

	AttributeSystem();
	AttributeSystem(const AttributeSet& baseAttributes);
	AttributeSystem(const AttributeMap& baseAttributesMap);

	float getAdjustedValue(Attribute id) const;
};


#endif /* Attributes_hpp */
