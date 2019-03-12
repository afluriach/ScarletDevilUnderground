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

#endif /* MiscMagicEffects_hpp */
