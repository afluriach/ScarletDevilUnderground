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

class StarbowBreak : public FirePattern
{
public:
	static constexpr int anglesCount = 5;

	static const float baseDamage;
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

	StarbowBreak(Agent *const agent);

	bullet_properties generateProps(int angle);
	bool spawnBullet(int angle, bool left);

	virtual bool fire();
	inline virtual bool fireIfPossible() { return fire(); }
	virtual bool spawn(SpaceFloat angle) { return false; }
	virtual bool spawn(SpaceVect posOffset, SpaceFloat angle) { return false; }
	virtual string iconPath() const { return ""; }

	virtual void update();
	//unused
	inline virtual float getCooldownTime() { return 0.0f; }
protected:
	array<float, anglesCount> timers = {};
};

class ScarletDaggerPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<ScarletDagger>
{
public:
	ScarletDaggerPattern(Agent *const agent);

	inline virtual string iconPath() const { return ""; }
	inline virtual float getCooldownTime() { return 2.0f / 3.0f; }
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

class FlandreWhirlShotPattern : public RadiusPattern, public FirePatternImpl<FlandrePolarMotionOrb>
{
public:
	FlandreWhirlShotPattern(Agent *const agent);

	virtual float getCooldownTime() { return 1.0f; }

	inline virtual string iconPath() const { return ""; }
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
