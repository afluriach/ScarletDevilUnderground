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
