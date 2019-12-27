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
	static bool canApply(GObject* target, float magnitude, float length, Attribute attr);

	RestoreAttribute(effect_params params, float magnitude, float length, Attribute attr);

	virtual void init();

	const Attribute attr;
};

class FortifyAttribute : public MagicEffect
{
public:
	FortifyAttribute(effect_params params, float magnitude, float length, Attribute attr);

	virtual void init();
	virtual void end();

	const Attribute attr;
};

//Uses currentSpeed attribute to determine amount of movement, applied per frame.
class DrainFromMovement : public MagicEffect
{
public:
	DrainFromMovement(effect_params params, float magnitude, float length, Attribute attr);

	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	Agent* agent;
	Attribute attr;
	//This ratio converts currentSpeed (m/s) to attribute units per frame.
	float _ratio;
};

#endif /* RestoreAttribute_hpp */
