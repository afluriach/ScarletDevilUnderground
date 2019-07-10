//
//  EnemyFirePattern.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef EnemyFirePattern_hpp
#define EnemyFirePattern_hpp

#include "Bullet.hpp"
#include "FirePattern.hpp"

class IceFairyBulletPattern : public SingleBulletFixedIntervalPattern, public BulletImplPattern
{
public:
	IceFairyBulletPattern(Agent *const agent);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }

	inline virtual float getCooldownTime() { return 2.0f; }
};

class Fairy1BulletPattern : public MultiBulletSpreadPattern, public BulletImplPattern
{
public:
	Fairy1BulletPattern(
		Agent *const agent,
		float fireInterval,
		SpaceFloat sideAngleSpread,
		int bulletCount
	);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }

	inline virtual float getCooldownTime() { return 1.5f; }
};

class GreenFairyBulletPattern : public RadiusPattern, public BulletImplPattern
{
public:
	GreenFairyBulletPattern(
		Agent *const agent,
		float fireInterval,
		int bulletCount
	);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }

	inline virtual float getCooldownTime() { return 1.5f; }
};

class BlueFairyFirePattern : public BurstPattern, public BulletImplPattern
{
public:
	BlueFairyFirePattern(Agent *const agent);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }
};

class ReimuWavePattern : public FirePattern
{
public:
	static const SpaceFloat omega;
	static const SpaceFloat amplitude;

	static SpaceVect parametric_move(
		SpaceFloat t,
		SpaceFloat angle,
		SpaceFloat phaseAngleStart,
		SpaceFloat speed
	);

	static parametric_space_function getParametricFunction(
		SpaceVect origin,
		SpaceFloat angle,
		SpaceFloat speed,
		SpaceFloat tOffset
	);

	ReimuWavePattern(Agent *const agent);
	virtual bool fire();

	//unused
	inline virtual bool spawn(SpaceFloat angle) { return false; }
	inline virtual bool spawn(SpaceVect posOffset, SpaceFloat angle) { return false; }

	inline virtual float getCooldownTime() { return 0.75f; }
	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }
protected:
	shared_ptr<bullet_properties> props;
};

class RumiaBurstPattern : public BurstPattern, public BulletImplPattern
{
public:
	RumiaBurstPattern(Agent *const agent);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }
};

class RumiaBurstPattern2 : public BurstPattern, public BulletImplPattern
{
public:
	RumiaBurstPattern2(Agent *const agent);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }
};


#endif /* EnemyFirePattern_hpp */
