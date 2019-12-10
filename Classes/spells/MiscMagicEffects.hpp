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
class TeleportPad;

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

//Uses currentSpeed attribute to determine amount of movement, applied per frame.
class DrainFromMovement : public MagicEffect
{
public:
	DrainFromMovement(Agent* agent, Attribute attr, float unitsPerMeter);

	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	Agent* agent;
	Attribute attr;
	//This ratio converts currentSpeed (m/s) to attribute units per frame.
	float _ratio;
};

class Teleport : public MagicEffect {
public:
	Teleport(GObject* target);
	inline virtual ~Teleport() {}

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	vector<object_ref<TeleportPad>> targets;
	object_ref<TeleportPad> toUse;
};

#endif /* MiscMagicEffects_hpp */
