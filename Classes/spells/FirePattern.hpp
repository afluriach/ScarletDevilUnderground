//
//  FirePattern.hpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#ifndef FirePattern_hpp
#define FirePattern_hpp

class Agent;

class FirePattern
{
public:
	inline FirePattern(Agent *const agent) : agent(agent) {}

	virtual bool fireIfPossible() = 0;
	virtual bool isInCooldown() = 0;
	virtual void update() = 0;

	virtual string iconPath() const = 0;
protected:
	Agent * const agent;
};

class SingleBulletFixedIntervalPattern : public FirePattern
{
public:
	inline SingleBulletFixedIntervalPattern(Agent *const agent) : FirePattern(agent) {}

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();

	void fire();

	virtual SpaceFloat getLaunchDistance() const { return 1.0; }
	virtual boost::rational<int> getCooldownTime() = 0;
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos) = 0;
protected:
	boost::rational<int> cooldownTimeRemaining = 0;
};

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

class IceFairyBulletPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline IceFairyBulletPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	//not relevant for enemy fire patterns
	virtual string iconPath() const { return ""; }

	virtual boost::rational<int> getCooldownTime() { return 1; }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

#endif /* FirePattern_hpp */
