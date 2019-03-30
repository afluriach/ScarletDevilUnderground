//
//  PlayerFirePattern.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef PlayerFirePattern_hpp
#define PlayerFirePattern_hpp

#include "FirePattern.hpp"
#include "PlayerBullet.hpp"

typedef BulletImplPattern<PlayerBulletImpl> PlayerBulletImplPattern;

enum class PlayerFirePatternID
{
	starbowBreak,
	catadioptric,
	scarletDagger,
};

class StarbowBreak : public FirePattern
{
public:
	static constexpr int anglesCount = 5;

	static const float baseDamage;
	static const float baseCost;
	static const float baseFireInterval;
	static const array<float, anglesCount> angleIntervalScales;

	static const double baseMass;
	static const double baseSpeed;

	static const double angleVariation;
	static const double angleStep;

	static const double launchDist;
	static const double baseRadius;
	static const array<double, anglesCount> radiusScales;

	static const array<Color3B, anglesCount> colors;

	StarbowBreak(Agent *const agent, int level);

	bullet_properties generateProps(int angle);
	bool spawnBullet(int angle, bool left);

	virtual bool fire();
	inline virtual bool fireIfPossible() { return fire(); }
	inline virtual bool spawn(SpaceFloat angle) { return false; }
	inline virtual bool spawn(SpaceVect posOffset, SpaceFloat angle) { return false; }
	inline virtual string iconPath() const { return "sprites/ui/starbow_break.png"; }

	virtual void update();
	//unused
	inline virtual float getCooldownTime() { return 0.0f; }
protected:
	array<float, anglesCount> timers = {};
};

class Catadioptric : public FirePattern
{
public:
	static const int tailCount;
	static const int secondaryBulletCount;
	static const int tertiaryBulletCount;
	static const SpaceFloat secondarySpeedVariation;
	static const SpaceFloat tertiarySpeedVariation;
	static const SpaceFloat angleSpread;

	Catadioptric(Agent *const agent, int level);

	bool spawnTail(SpaceFloat angleOffset);

	virtual bool fire();
	inline virtual bool spawn(SpaceFloat angle) { return false; }
	inline virtual bool spawn(SpaceVect posOffset, SpaceFloat angle) { return false; }
	inline virtual float getCooldownTime() { return 1.5f; }
	inline virtual string iconPath() const { return "sprites/ui/catadioptric.png"; }
	inline virtual float getCost() const { return 7.5f; }
};

class ScarletDaggerPattern : public MultiBulletSpreadPattern, public FirePatternImpl<ScarletDagger>
{
public:
	struct properties
	{
		SpaceFloat spreadAngle;
		float fireInterval;
		int bulletCount;
	};

	static constexpr int levelsCount = 3;
	static const array<properties, levelsCount> props;

	ScarletDaggerPattern(Agent *const agent, int level);

	inline virtual string iconPath() const { return "sprites/scarlet_dagger.png"; }
	inline virtual float getCooldownTime() { return props[level].fireInterval; }
	inline virtual float getCost() const { return 4.0f; }
protected:
	int level;
};

class FlandreBigOrbPattern : public SingleBulletFixedIntervalPattern, public PlayerBulletImplPattern
{
public:
	FlandreBigOrbPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_big_orb.png"; }

	virtual float getCooldownTime() { return 1.0f; }
};

class FlandreFastOrbPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<FlandreFastOrb1>
{
public:
	FlandreFastOrbPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }

	virtual float getCooldownTime() { return 1.0f / 6.0f; }
};

class FlandreWideAnglePattern1 : public MultiBulletSpreadPattern, public FirePatternImpl<FlandreFastOrb1>
{
public:
	static const float cooldown;

	FlandreWideAnglePattern1(Agent *const agent);

	inline virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }
};

class FlandreWideAnglePattern2 : public MultiBulletSpreadPattern, public FirePatternImpl<FlandreFastOrb1>
{
public:
	static const float cooldown;

	FlandreWideAnglePattern2(Agent *const agent);

	inline virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }
};

class RumiaFastOrbPattern : public SingleBulletFixedIntervalPattern, public PlayerBulletImplPattern
{
public:
	RumiaFastOrbPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/rumia_fast_orb.png"; }

	virtual float getCooldownTime() { return 1.0f / 6.0f; }
};

class RumiaParallelPattern : public MultiBulletParallelPattern, public PlayerBulletImplPattern
{
public:
	static const float cooldown;

	RumiaParallelPattern(Agent *const agent);

	inline virtual string iconPath() const { return "sprites/fire_patterns/rumia_fast_orb.png"; }
};

class CirnoLargeIceBulletPattern : public SingleBulletFixedIntervalPattern, public PlayerBulletImplPattern
{
public:
	CirnoLargeIceBulletPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }

	virtual float getCooldownTime() { return 1.0f; }
};

class CirnoSmallIceBulletPattern : public SingleBulletFixedIntervalPattern, public PlayerBulletImplPattern
{
public:
	CirnoSmallIceBulletPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }

	virtual float getCooldownTime() { return 0.25f; }
};

#endif /* PlayerFirePattern_hpp */
