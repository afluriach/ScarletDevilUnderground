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

class FlandreBigOrbPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<FlandreBigOrb1>
{
public:
	FlandreBigOrbPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_big_orb.png"; }

	virtual boost::rational<int> getCooldownTime() { return 1; }
};

class FlandreFastOrbPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<FlandreFastOrb1>
{
public:
	FlandreFastOrbPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }

	virtual boost::rational<int> getCooldownTime() { return boost::rational<int>(1,6); }
};

class FlandreWideAnglePattern : public MultiBulletSpreadPattern, public FirePatternImpl<FlandreFastOrb1>
{
public:
	static const boost::rational<int> cooldown;

	FlandreWideAnglePattern(Agent *const agent);

	inline virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }
};

class RumiaFastOrbPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<RumiaFastOrb1>
{
public:
	RumiaFastOrbPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/rumia_fast_orb.png"; }

	virtual boost::rational<int> getCooldownTime() { return boost::rational<int>(1, 6); }
};

class RumiaParallelPattern : public FirePatternImpl<RumiaFastOrb1>
{
public:
	static const boost::rational<int> cooldown;

	RumiaParallelPattern(Agent *const agent);

	inline virtual string iconPath() const { return "sprites/fire_patterns/rumia_fast_orb.png"; }
	inline virtual boost::rational<int> getCooldownTime() { return cooldown; }

	virtual bool fire();
};

class CirnoLargeIceBulletPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<CirnoLargeIceBullet>
{
public:
	CirnoLargeIceBulletPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }

	virtual boost::rational<int> getCooldownTime() { return 1; }
};

class CirnoSmallIceBulletPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<CirnoSmallIceBullet>
{
public:
	CirnoSmallIceBulletPattern(Agent *const agent);

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }

	virtual boost::rational<int> getCooldownTime() { return boost::rational<int>(1,4); }
};

#endif /* PlayerFirePattern_hpp */
