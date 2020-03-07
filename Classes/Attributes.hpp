//
//  Attributes.hpp
//  Koumachika
//
//  Created by Toni on 12/3/18.
//
//

#ifndef Attributes_hpp
#define Attributes_hpp

#define hp_damage_map(x) AttributeMap{ {Attribute::hp, -x} }

enum class Attribute {
	begin = 0,

	hp = 0,
	mp,
	stamina,

	maxHP,
	maxMP,
	maxStamina,

	hpRegen,
	mpRegen,
	staminaRegen,

	hpInv,
	mpInv,
	staminaInv,

	hpRatio,
	mpRatio,
	staminaRatio,

	inhibitFiring,
	inhibitMovement,
	inhibitSpellcasting,

	hitProtection,
	invisibility,

	combo,

	attack,
	attackSpeed,
	bulletSpeed,

	shieldLevel,

	stress,
	stressDecay,
	stressFromHits,
	stressFromBlocks,
	stressFromDetects,

	agility,

	currentSpeed,
	speedRatio,
	maxSpeed,
	maxAcceleration,

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

struct IncidentAttributeEntry
{
	Attribute current;
	Attribute maximum;
	Attribute regeneration;

	//Derived
	Attribute maximum_inverse;
	Attribute ratio;
};

struct UpgradeInfo
{
	float step;
	string sprite;
};

typedef array<float, to_size_t(Attribute::end)> AttributeArray;

class AttributeSystem
{
public:
	static constexpr IncidentAttributeEntry hp = { Attribute::hp, Attribute::maxHP, Attribute::hpRegen, Attribute::hpInv, Attribute::hpRatio};
	static constexpr IncidentAttributeEntry mp = { Attribute::mp, Attribute::maxMP, Attribute::mpRegen, Attribute::mpInv, Attribute::mpRatio };
	static constexpr IncidentAttributeEntry stamina = { Attribute::stamina, Attribute::maxStamina, Attribute::staminaRegen, Attribute::staminaInv, Attribute::staminaRatio };

	static pair<float, float> calculateAgilityAttributes(float agility);

	static AttributeMap scale(const AttributeMap& input, float scale);
	static AttributeMap add(const AttributeMap& a, const AttributeMap& b);
	static float getAttribute(const AttributeMap& attr, Attribute id, float default = 0.0f);
	static AttributeArray getAttributeSet(const AttributeMap& input);
	static AttributeMap getAttributeMap(Attribute id, float val);
	static AttributeMap getAttributeElementMap(Attribute element, float damage, float elementScale = 1.0f);
	static AttributeArray getBlankAttributeSet();
	static AttributeArray getZeroAttributeSet();
	static Attribute getElementSensitivity(Attribute element);
	static Attribute getElement(Attribute elementSensitivity);

	static Attribute getAttribute(const string& name);

	static const float maxElementDamage;
	static const float maxComboPoints;
	static const float comboPointsDrainPerSecond;

	static constexpr size_t upgradeCount = 8;

	static const boost::bimap<Attribute, string> attributeNameMap;

	AttributeSystem();
	AttributeSystem(const AttributeArray& baseAttributes);
	AttributeSystem(const AttributeMap& baseAttributesMap);

	float operator[](Attribute id) const;
	float get(string name) const;
	void set(string name, float val);
	void set(Attribute id, float val);

	void increment(Attribute a);
	void decrement(Attribute a);

	void update(Agent* agent);
	void applyIncidentRegen(IncidentAttributeEntry entry);
	void applyElementDecay();

	float getIncidentRatio(IncidentAttributeEntry entry) const;
	float getHealthRatio() const;
	float getMagicRatio() const;
	float getStaminaRatio() const;

	float applyDamage(DamageInfo damage);
	void apply(const AttributeMap& effects);
	float getAttackMultiplier() const;
	float getTypeSensitivity(DamageType type) const;
	bool canApplyIncidentAttribute(IncidentAttributeEntry entry) const;
	void modifyIncidentAttribute(IncidentAttributeEntry entry, float x);
	void modifyIncidentAttributeMaximum(IncidentAttributeEntry entry, float x);
	void setIncidentMaxInv(IncidentAttributeEntry entry);
	void applyElementalDamage(Attribute id, Attribute maxID, float x);
	bool canApplyAttribute(Attribute id, float x);
	void modifyAttribute(Attribute id, float x);
	void modifyAttribute(Attribute mod, Attribute addend, float scale = 1.0f);
	void timerDecrement(Attribute id, float scale = 1.0f);
	void timerIncrement(Attribute id, Attribute maxID, float scale);
	bool isZero(Attribute id) const;
	bool isNonzero(Attribute id) const;
	void setProtection();

	void setFull(IncidentAttributeEntry entry);
	void setEmpty(IncidentAttributeEntry entry);
	void setFullHP();
	void setFullMP();
	void setEmptyMP();
	void setFullStamina();
	void setEmptyStamina();
	void resetCombo();

	void modifyAgility(float dx);

	float getWithinRange(float input, float min, float max);
protected:
	AttributeArray attributes;
};


#endif /* Attributes_hpp */
