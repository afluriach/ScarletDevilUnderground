//
//  FirePattern.hpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#ifndef FirePattern_hpp
#define FirePattern_hpp

#include "Agent.hpp"

class FirePattern
{
public:
	inline FirePattern(Agent *const agent) : agent(agent) {}
	virtual inline ~FirePattern() {}

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();
	virtual SpaceFloat getLaunchDistance() const { return 1.0; }

	virtual bool fire() = 0;
	virtual GObject::GeneratorType spawn(SpaceFloat angle) = 0;
	virtual GObject::GeneratorType spawn(SpaceVect posOffset, SpaceFloat angle) = 0;
	virtual float getCooldownTime() = 0;
	virtual string iconPath() const = 0;
protected:
	float cooldownTimeRemaining = 0;
	Agent * const agent;
};

template<class C>
class FirePatternImpl : virtual public FirePattern
{
public:
	inline FirePatternImpl() {}
	virtual inline ~FirePatternImpl() {}

	inline virtual GObject::GeneratorType spawn(SpaceFloat angle)
	{
		return GObject::make_object_factory<C>(
			agent,
			angle,
			agent->getPos()+ SpaceVect::ray(getLaunchDistance(), angle)
		);
	}

	inline virtual GObject::GeneratorType spawn(SpaceVect posOffset, SpaceFloat angle)
	{
		return GObject::make_object_factory<C>(
			agent,
			angle,
			agent->getPos() + SpaceVect::ray(getLaunchDistance(), angle) + posOffset
		);
	}

};

class SingleBulletFixedIntervalPattern : virtual public FirePattern
{
public:
	inline SingleBulletFixedIntervalPattern(Agent *const agent) : FirePattern(agent) {}
	virtual inline ~SingleBulletFixedIntervalPattern() {}

	virtual bool fire();
};

class RadiusPattern : virtual public FirePattern
{
public:
	RadiusPattern(
		Agent *const agent,
		float fireInterval,
		int bulletCount
	);
	virtual inline ~RadiusPattern() {}

	virtual bool fire();
	inline virtual float getCooldownTime() { return fireInterval; }
protected:
	int bulletCount;
	float fireInterval;
};

class MultiBulletSpreadPattern : virtual public FirePattern
{
public:
	MultiBulletSpreadPattern(
		Agent *const agent,
		float fireInterval, 
		SpaceFloat sideAngleSpread,
		int bulletCount
	);
	virtual inline ~MultiBulletSpreadPattern() {}

	virtual bool fire();
	inline virtual float getCooldownTime() { return fireInterval; }
protected:
	SpaceFloat sideAngleSpread;
	int bulletCount;
	float fireInterval;
};

class MultiBulletParallelPattern : virtual public FirePattern
{
public:
	MultiBulletParallelPattern(
		Agent *const agent,
		float fireInterval,
		SpaceFloat bulletSpacing,
		int bulletCount
	);
	virtual inline ~MultiBulletParallelPattern() {}

	virtual bool fire();
	inline virtual float getCooldownTime() { return fireInterval; }
protected:
	SpaceFloat bulletSpacing;
	int bulletCount;
	float fireInterval;
};


#endif /* FirePattern_hpp */
