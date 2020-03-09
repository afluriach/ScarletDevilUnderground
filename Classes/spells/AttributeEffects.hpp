//
//  AttributeEffects.hpp
//  Koumachika
//
//  Created by Toni on 12/18/19.
//
//

#ifndef RestoreAttribute_hpp
#define RestoreAttribute_hpp

#include "MagicEffect.hpp"

class RestoreAttribute : public MagicEffect
{
public:
	static constexpr effect_flags flags = enum_bitwise_or(effect_flags, immediate, agent);

	static bool canApply(GObject* target, effect_attributes attrs, Attribute attr);

	RestoreAttribute(effect_params params, Attribute attr);

	virtual void init();

	const Attribute attr;
};

class FortifyAttribute : public MagicEffect
{
public:
	static constexpr effect_flags flags = enum_bitwise_or(effect_flags, durable, agent);

	FortifyAttribute(effect_params params, Attribute attr);

	virtual void init();
	virtual void end();

	const Attribute attr;
};

//Uses currentSpeed attribute to determine amount of movement, applied per frame.
class DrainFromMovement : public MagicEffect
{
public:
	static constexpr effect_flags flags = enum_bitwise_or3(effect_flags, durable, active, agent);

	DrainFromMovement(effect_params params, Attribute attr);

	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	Agent* agent;
	Attribute attr;
	//This ratio converts currentSpeed (m/s) to attribute units per frame.
	float _ratio;
};

//Increments Attribute while the effect is active.
class SetBoolAttribute : public MagicEffect
{
public:
	static constexpr effect_flags flags = enum_bitwise_or(effect_flags, durable, agent);

	SetBoolAttribute(effect_params params, Attribute attr);

	virtual void init();
	virtual void end();

	const Attribute attr;
};

class DamageOverTime : public MagicEffect
{
public:
	static constexpr effect_flags flags = enum_bitwise_or(effect_flags, durable, active);

	DamageOverTime(effect_params params, Attribute element);

	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	Attribute element;
};

#endif /* RestoreAttribute_hpp */
