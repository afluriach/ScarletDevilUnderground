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

enum class PlayerFirePatternID : int;

class FirePattern
{
public:
	static const unordered_map<string, FirePatternGeneratorType> playerFirePatterns;

	inline FirePattern(Agent *const agent) : agent(agent) {}
	virtual inline ~FirePattern() {}

	GSpace* getSpace() const;

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();
	virtual SpaceFloat getLaunchDistance() const { return 1.0; }

	virtual bool fire() = 0;
	virtual bool spawn(SpaceFloat angle) = 0;
	virtual bool spawn(SpaceVect posOffset, SpaceFloat angle) = 0;
	virtual float getCooldownTime() = 0;
	virtual string iconPath() const = 0;
	inline virtual float getCost() const { return 0.0f; }
protected:
	float cooldownTimeRemaining = 0;
	Agent * const agent;
};

template<class C>
class BulletImplPattern : virtual public FirePattern
{
public:
	inline BulletImplPattern(shared_ptr<bullet_properties> props) : props(props) {}
	virtual inline ~BulletImplPattern() {}

	inline virtual bool spawn(SpaceFloat angle)
	{
		return spawn(SpaceVect::zero, angle);
	}

	inline virtual bool spawn(SpaceVect posOffset, SpaceFloat angle)
	{
		SpaceVect _pos = agent->getPos() + SpaceVect::ray(getLaunchDistance(), angle) + posOffset;

		return agent->bulletImplCheckSpawn<C>(
			Bullet::makeParams(_pos, angle),
			props
		).isFuture();
	}

	shared_ptr<bullet_properties> props;
};

template<class C>
class FirePatternImpl : virtual public FirePattern
{
public:
	inline FirePatternImpl() {}
	virtual inline ~FirePatternImpl() {}

	inline virtual bool spawn(SpaceFloat angle)
	{
		return spawn(SpaceVect::zero , angle);
	}

	inline virtual bool spawn(SpaceVect posOffset, SpaceFloat angle)
	{
		SpaceVect _pos = agent->getPos() + SpaceVect::ray(getLaunchDistance(), angle) + posOffset;

		return agent->bulletCheckSpawn<C>(
			Bullet::makeParams(_pos, angle)
		).isFuture();
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

class BurstPattern : virtual public FirePattern
{
public:
	BurstPattern(
		Agent *const agent,
		float burstInterval,
		float burstLength,
		int bulletsPerBurst
	);
	virtual inline ~BurstPattern() {}

	virtual bool isInCooldown();
	virtual void update();
	virtual bool fire();

	inline virtual float getCooldownTime() { return 0.0f; }
protected:
	int crntBurstCount = 0 ;
	float countdownTimer = 0.0f;

	const float burstInterval;
	const float burstLength;
	const int bulletsPerBurst;
};


#endif /* FirePattern_hpp */
