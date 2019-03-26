//
//  MiscMagicEffects.hpp
//  Koumachika
//
//  Created by Toni on 3/12/19.
//
//

#ifndef MiscMagicEffects_hpp
#define MiscMagicEffects_hpp

#include "MagicEffect.hpp"

class Agent;
class RedFairy;

class FreezeStatusEffect : public MagicEffect
{
public:
	FreezeStatusEffect(gobject_ref target);

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	float timer = 0.0f;
};

class DarknessCurseEffect : public MagicEffect
{
public:
	DarknessCurseEffect(gobject_ref target);

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	Agent* agent;
};

class RedFairyStress : public MagicEffect
{
public:
	RedFairyStress(object_ref<Agent> _agent);

	inline virtual void init() {}
	virtual void update();
	virtual void end();
protected:
	float baseAttackSpeed;
	Agent * agent;
};

class GhostProtection : public MagicEffect
{
public:
	GhostProtection(object_ref<Agent> _agent);

	virtual void init();
	virtual void update();
	inline virtual void end() {}
protected:
	float accumulator = 0.0f;
	float crntHP;
	Agent * agent;
};

class BulletSpeedFromHP : public MagicEffect
{
public:
	BulletSpeedFromHP(
		object_ref<Agent> _agent,
		float_pair debuffRange,
		float_pair buffRange,
		float maxDebuff,
		float maxBuff
	);

	inline virtual void init() {}
	virtual void update();
	virtual void end();
protected:
	float baseBulletSpeed;
	float_pair debuffRange;
	float_pair buffRange;
	float maxDebuff;
	float maxBuff;
	Agent * agent;
};


#endif /* MiscMagicEffects_hpp */
