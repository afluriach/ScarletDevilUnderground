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

class RadiusEffect : public MagicEffect
{
public:
	RadiusEffect(GObject* agent, DamageInfo damage, SpaceFloat radius, GType type);

	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	DamageInfo damage;
	SpaceFloat radius;
	GType type;
};

class FreezeStatusEffect : public MagicEffect
{
public:
	FreezeStatusEffect(GObject* agent);

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	float timer = 0.0f;
};

class DarknessCurseEffect : public MagicEffect
{
public:
	DarknessCurseEffect(Agent* agent);

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	Agent* agent;
};

class RedFairyStress : public MagicEffect
{
public:
	RedFairyStress(Agent* agent);

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
	GhostProtection(Agent* agent);

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
		Agent* agent,
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
