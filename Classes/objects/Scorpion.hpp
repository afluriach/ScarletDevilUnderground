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

class Scorpion1 :
public Enemy,
public BaseAttributes<Scorpion1>
{
public:
	static const AttributeMap baseAttributes;
	static const string properName;

	Scorpion1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual DamageInfo touchEffect() const;

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 10.0; }

	inline string getSprite() const { return "scorpion"; }

	virtual void initStateMachine();
};

class Scorpion2 :
	public Enemy,
	public BaseAttributes<Scorpion2>
{
public:
	static const AttributeMap baseAttributes;
	static const string properName;

	Scorpion2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual DamageInfo touchEffect() const;

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 10.0; }

	inline string getSprite() const { return "scorpion2"; }

	virtual void initStateMachine();
};


#endif /* Slime_hpp */
