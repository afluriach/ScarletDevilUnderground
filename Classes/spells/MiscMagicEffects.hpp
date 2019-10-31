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

class DrainStaminaFromMovement : public MagicEffect
{
public:
	DrainStaminaFromMovement(Agent* agent);

	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	Agent* agent;
};

#endif /* MiscMagicEffects_hpp */
