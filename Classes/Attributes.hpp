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
	hitProtection,
	spellCooldown,

	maxHP,
	maxMP,
	maxPower,
	hitProtectionInterval,
	spellCooldownInterval,

	stamina,
	maxStamina,

	speed,
	acceleration,

	iceSensitivity,
	sunSensitivity,
	poisonSensitivity,
	slimeSensitivity,

	iceDamage,
	sunDamage,
	poisonDamage,
	slimeDamage,

	end,
};

typedef array<float, to_size_t(Attribute::end)> AttributeSet;
typedef map<Attribute, float> AttributeMap;

class AttributeSystem
{
public:
	static AttributeMap scale(const AttributeMap& input, float scale);
	static AttributeSet getAttributeSet(const AttributeMap& input);
	static AttributeMap getAttributeMap(Attribute id, float val);
	static AttributeSet getBlankAttributeSet();
	AttributeSet getZeroAttributeSet();

	static const float maxElementDamage;

	AttributeSystem();
	AttributeSystem(const AttributeSet& baseAttributes);
	AttributeSystem(const AttributeMap& baseAttributesMap);

	float getAdjustedValue(Attribute id) const;
	void modifyIncidentAttribute(Attribute id, Attribute maxID, float x);
	void applyElementalDamage(Attribute id, Attribute maxID, float x);
	bool canApplyAttribute(Attribute id, float x);
	void modifyAttribute(Attribute id, float x);
	void timerDecrement(Attribute id);
	bool isNonzero(Attribute id);
	void setProtection();
	void setTimedProtection(float seconds);
	void resetProtection();

	void setFullHP();
	void setFullMP();
	void setEmptyMP();
	void setStartMP();
	void setFullPower();
	void setEmptyPower();
	void setStartPower();
	void setHitProtection();
	void setSpellCooldown();

	void setFullStamina();

	float getWithinRange(float input, float min, float max);
protected:
	AttributeSet attributes;
};


#endif /* Attributes_hpp */
