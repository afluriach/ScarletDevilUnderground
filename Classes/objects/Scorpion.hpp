//
//  Scorpion.hpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#ifndef Scorpion_hpp
#define Scorpion_hpp

#include "Enemy.hpp"
#include "Collectibles.hpp"
#include "macros.h"

class Scorpion1 :
public Enemy,
public BaseAttributes<Scorpion1>
{
public:
	static const AttributeMap baseAttributes;

	Scorpion1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual AttributeMap touchEffect();

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 10.0; }

	inline string imageSpritePath() const { return "sprites/scorpion.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Scorpion2 :
	public Enemy,
	public BaseAttributes<Scorpion2>
{
public:
	static const AttributeMap baseAttributes;

	Scorpion2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual AttributeMap touchEffect();

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 10.0; }

	inline string imageSpritePath() const { return "sprites/scorpion2.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};


#endif /* Slime_hpp */
