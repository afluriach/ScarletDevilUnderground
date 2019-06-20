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

class Slime1 :
public Enemy,
public BaseAttributes<Slime1>
{
public:
	static const string baseAttributes;
	static const string properName;

	Slime1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual DamageInfo touchEffect() const;

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string getSprite() const { return "slime"; }

	virtual void initStateMachine();
};

class Slime2 :
	public Enemy,
	public BaseAttributes<Slime2>
{
public:
	static const string baseAttributes;
	static const string properName;

	Slime2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual DamageInfo touchEffect() const;

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 200.0; }

	inline string getSprite() const { return "slime"; }

	virtual void initStateMachine();
};


#endif /* Slime_hpp */
