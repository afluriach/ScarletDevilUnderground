//
//  Slime.hpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#ifndef Slime_hpp
#define Slime_hpp

#include "Enemy.hpp"
#include "macros.h"

class Slime1 :
public Enemy,
public BaseAttributes<Slime1>
{
public:
	static const AttributeMap baseAttributes;

	Slime1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual AttributeMap touchEffect();

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	virtual inline SpaceFloat getRadius() const { return 0.875; }
	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/slime.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 128; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Slime2 :
	public Enemy,
	public BaseAttributes<Slime2>
{
public:
	static const AttributeMap baseAttributes;

	Slime2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual AttributeMap touchEffect();

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	virtual inline SpaceFloat getRadius() const { return 1.75; }
	inline SpaceFloat getMass() const { return 200.0; }

	inline string imageSpritePath() const { return "sprites/slime.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 64; }

	virtual void initStateMachine(ai::StateMachine& sm);
};


#endif /* Slime_hpp */
