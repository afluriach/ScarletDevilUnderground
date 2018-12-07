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
	virtual void fire() = 0;
	virtual bool isInCooldown() = 0;
	virtual void update() = 0;

	virtual string iconPath() const = 0;
	virtual int powerCost() const = 0;
protected:
	Agent * const agent;
};

class SingleBulletFixedIntervalPattern : public FirePattern
{
public:
	inline SingleBulletFixedIntervalPattern(Agent *const agent) : FirePattern(agent) {}

	virtual bool fireIfPossible();
	virtual void fire();
	virtual bool isInCooldown();
	virtual void update();

	virtual float getLaunchDistance() const { return 1.0f; }
	virtual int getCooldownFrames() = 0;
	virtual GObject::GeneratorType spawn(float angle, SpaceVect pos) = 0;
protected:
	int cooldownFramesRemaining = 0;
};

class FlandreBigOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline FlandreBigOrbPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_big_orb.png"; }
	virtual int powerCost() const { return 5; }

	virtual inline int getCooldownFrames() { return 60; }
	virtual GObject::GeneratorType spawn(float angle, SpaceVect pos);
protected:
	int cooldownFramesRemaining = 0;
};

class FlandreFastOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline FlandreFastOrbPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }
	virtual int powerCost() const { return 1; }

	virtual inline int getCooldownFrames() { return 10; }
	virtual GObject::GeneratorType spawn(float angle, SpaceVect pos);
protected:
	int cooldownFramesRemaining = 0;
};

class CirnoLargeIceBulletPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline CirnoLargeIceBulletPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	virtual string iconPath() const { return "sprites/fire_patterns/cirno_large_ice_bullet.png"; }
	virtual int powerCost() const { return 5; }

	virtual inline int getCooldownFrames() { return 60; }
	virtual GObject::GeneratorType spawn(float angle, SpaceVect pos);
protected:
	int cooldownFramesRemaining = 0;
};

class IceFairyBulletPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline IceFairyBulletPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	//not relevant for enemy fire patterns
	virtual string iconPath() const { return ""; }
	virtual int powerCost() const { return 0; }

	virtual inline int getCooldownFrames() { return 60; }
	virtual GObject::GeneratorType spawn(float angle, SpaceVect pos);
protected:
	int cooldownFramesRemaining = 0;
};



#endif /* FirePattern_hpp */
