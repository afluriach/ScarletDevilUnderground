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
	begin = 0,
	hp = 0,
	mp,
	power,

	maxHP,
	maxMP,
	maxPower,

	speed,
	acceleration,

	hitProtection,
	iceSensitivity,

	iceDamage,

	end,
};

typedef array<float, to_size_t(Attribute::end)> AttributeSet;
typedef map<Attribute, float> AttributeMap;

AttributeSet getAttributeSet(const AttributeMap& input);
AttributeMap getAttributeMap(Attribute id, float val);
AttributeSet getBlankAttributeSet();

extern const float maxElementDamage;

class AttributeSystem
{
public:
	AttributeSet attributes;

	AttributeSystem();
	AttributeSystem(const AttributeSet& baseAttributes);
	AttributeSystem(const AttributeMap& baseAttributesMap);

	float getAdjustedValue(Attribute id) const;
	void modifyAttribute(Attribute id, float x);

	void setFullHP();
	void setFullMP();
	void setEmptyMP();
	void setFullPower();
	void setEmptyPower();

	float getWithinRange(float input, float min, float max);
};


#endif /* Attributes_hpp */
