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

class FlandreBigOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline FlandreBigOrbPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_big_orb.png"; }

	virtual boost::rational<int> getCooldownTime() { return 1; }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

class FlandreFastOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline FlandreFastOrbPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }

	virtual boost::rational<int> getCooldownTime() { return boost::rational<int>(1,6); }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

class FlandreWideAnglePattern : public FirePattern
{
public:
	static const boost::rational<int> primaryCooldown;
	static const boost::rational<int> sideCooldown;

	inline FlandreWideAnglePattern(Agent *const agent) : FirePattern(agent) {}

	inline virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();
protected:
	boost::rational<int> crntPrimaryCooldown;
	boost::rational<int> crntSideCooldown;
};

class RumiaFastOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline RumiaFastOrbPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/rumia_fast_orb.png"; }

	virtual boost::rational<int> getCooldownTime() { return boost::rational<int>(1, 6); }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

class RumiaParallelPattern : public FirePattern
{
public:
	static const boost::rational<int> cooldown;

	inline RumiaParallelPattern(Agent *const agent) : FirePattern(agent) {}

	inline virtual string iconPath() const { return "sprites/fire_patterns/rumia_fast_orb.png"; }

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();
protected:
	boost::rational<int> crntCooldown;
};

class CirnoLargeIceBulletPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline CirnoLargeIceBulletPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }

	virtual boost::rational<int> getCooldownTime() { return 1; }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

class CirnoSmallIceBulletPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline CirnoSmallIceBulletPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }

	virtual boost::rational<int> getCooldownTime() { return boost::rational<int>(1,4); }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

#endif /* PlayerFirePattern_hpp */
