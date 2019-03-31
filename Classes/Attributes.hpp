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

	touchDamage,

	attack,
	attackSpeed,
	bulletSpeed,

	ricochet,
	bulletCount,

	shieldLevel,

	stress,
	stressDecay,
	stressFromHits,
	stressFromBlocks,
	stressFromDetects,

	agility,
	speed,
	acceleration,

	bombSensitivity,
	bulletSensitivity,
	meleeSensitivity,

	beginElementSensitivity,
	iceSensitivity = beginElementSensitivity,
	sunSensitivity,
	darknessSensitivity,
	poisonSensitivity,
	slimeSensitivity,
	endElementSensitivity = slimeSensitivity,

	beginElementDamage,
	iceDamage = beginElementDamage,
	sunDamage,
	darknessDamage,
	poisonDamage,
	slimeDamage,

	endElementDamage,
	end = endElementDamage,
};

enum class DamageType
{
	bullet,
	bomb,
	effectArea,
	touch,
	melee,
	pitfall,

	end
};

struct UpgradeInfo
{
	float step;
	string sprite;
};

struct DamageInfo
{
	float mag = 0.0f;
	Attribute element = Attribute::end;
	DamageType type = DamageType::end;

	DamageInfo operator*(float rhs);
};

#define bullet_damage(x) DamageInfo{x, Attribute::end, DamageType::bullet}
#define bomb_damage(x) DamageInfo{x, Attribute::end, DamageType::bomb}
#define melee_damage(x) DamageInfo{x, Attribute::end, DamageType::melee}

typedef array<float, to_size_t(Attribute::end)> AttributeSet;
typedef map<Attribute, float> AttributeMap;
typedef map<string, AttributeMap> AttributePackageMap;

class AttributeSystem
{
public:
	static pair<float, float> calculateAgilityAttributes(float agility);

	static AttributeMap scale(const AttributeMap& input, float scale);
	static AttributeMap add(const AttributeMap& a, const AttributeMap& b);
	static float getAttribute(const AttributeMap& attr, Attribute id, float default = 0.0f);
	static AttributeSet getAttributeSet(const AttributeMap& input);
	static AttributeMap getAttributeMap(Attribute id, float val);
	static AttributeMap getAttributeElementMap(Attribute element, float damage, float elementScale = 1.0f);
	static AttributeSet getBlankAttributeSet();
	static AttributeSet getZeroAttributeSet();
	static Attribute getElementSensitivity(Attribute element);
	static Attribute getElement(Attribute elementSensitivity);

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

	float applyDamage(DamageInfo damage);
	void apply(const AttributeMap& effects);
	float getAttackMultiplier() const;
	float getTypeSensitivity(DamageType type) const;
	AttributeMap scaleBulletAttributes(const AttributeMap& bulletAttributes) const;
	void modifyIncidentAttribute(Attribute id, Attribute maxID, float x);
	void applyElementalDamage(Attribute id, Attribute maxID, float x);
	bool canApplyAttribute(Attribute id, float x);
	void modifyAttribute(Attribute id, float x);
	void modifyAttribute(Attribute mod, Attribute addend, float scale = 1.0f);
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
