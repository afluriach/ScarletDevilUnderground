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

#define hp_damage_map(x) AttributeMap{ {Attribute::hp, -x} }

enum class Attribute {
	begin = 0,

	hp = 0,
	mp,
	stamina,
	hitProtection,
	spellCooldown,

	maxHP,
	maxMP,
	maxStamina,
	hitProtectionInterval,
	spellCooldownInterval,

	hpRegen,
	mpRegen,
	staminaRegen,

	keys,
	combo,

	attack,
	attackSpeed,

	ricochet,
	bulletCount,

	shieldActive,
	shieldLevel,

	stress,
	stressDecay,

	agility,
	speed,
	acceleration,

	iceSensitivity,
	sunSensitivity,
	darknessSensitivity,
	poisonSensitivity,
	slimeSensitivity,

	beginElementDamage,
	iceDamage = beginElementDamage,
	sunDamage,
	darknessDamage,
	poisonDamage,
	slimeDamage,

	endElementDamage,
	end = endElementDamage,
};

struct UpgradeInfo
{
	float step;
	string sprite;
};

typedef array<float, to_size_t(Attribute::end)> AttributeSet;
typedef map<Attribute, float> AttributeMap;
typedef map<string, AttributeMap> AttributePackageMap;

class AttributeSystem
{
public:
	static pair<float, float> calculateAgilityAttributes(float agility);

	static AttributeMap scale(const AttributeMap& input, float scale);
	static AttributeMap add(const AttributeMap& a, const AttributeMap& b);
	static AttributeSet getAttributeSet(const AttributeMap& input);
	static AttributeMap getAttributeMap(Attribute id, float val);
	static AttributeMap getAttributeElementMap(Attribute element, float damage, float elementScale = 1.0f);
	static AttributeSet getBlankAttributeSet();
	static AttributeSet getZeroAttributeSet();

	static const float maxElementDamage;
	static const float maxComboPoints;
	static const float comboPointsDrainPerSecond;

	static constexpr size_t upgradeCount = 8;

	static const unordered_map<Attribute, UpgradeInfo> upgradeAttributes;
	static const boost::bimap<Attribute, string> attributeNameMap;

	AttributeSystem();
	AttributeSystem(const AttributeSet& baseAttributes);
	AttributeSystem(const AttributeMap& baseAttributesMap);

	float operator[](Attribute id) const;
	float get(string name) const;
	void set(string name, float val);
	void set(Attribute id, float val);
	void _set(int id, float val);

	void update();
	void applyIncidentRegen();
	void applyElementDecay();

	float getAttackMultiplier() const;
	AttributeMap scaleBulletAttributes(const AttributeMap& bulletAttributes) const;
	void modifyIncidentAttribute(Attribute id, Attribute maxID, float x);
	void applyElementalDamage(Attribute id, Attribute maxID, float x);
	bool canApplyAttribute(Attribute id, float x);
	void modifyAttribute(Attribute id, float x);
	void timerDecrement(Attribute id, float scale = 1.0f);
	void timerIncrement(Attribute id, Attribute maxID, float scale);
	bool isNonzero(Attribute id) const;
	void setProtection();
	void setTimedProtection(float seconds);
	void resetProtection();

	void setFullHP();
	void setFullMP();
	void setEmptyMP();
	void setFullStamina();
	void setEmptyStamina();
	void setHitProtection();
	void setSpellCooldown();
	void resetCombo();

	void modifyAgility(float dx);

	float getWithinRange(float input, float min, float max);
protected:
	AttributeSet attributes;
};


#endif /* Attributes_hpp */
