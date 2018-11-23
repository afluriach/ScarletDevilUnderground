//
//  FirePattern.hpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#ifndef FirePattern_hpp
#define FirePattern_hpp

class FirePattern
{
public:
	inline FirePattern(Player *const player) : player(player) {}

	virtual bool fireIfPossible() = 0;
	virtual void fire() = 0;
	virtual bool isInCooldown() = 0;
	virtual void update() = 0;

	virtual string iconPath() const = 0;
	virtual int powerCost() const = 0;
protected:
	Player * const player;
};

class SingleBulletFixedIntervalPattern : public FirePattern
{
public:
	inline SingleBulletFixedIntervalPattern(Player *const player) : FirePattern(player) {}

	virtual bool fireIfPossible();
	virtual void fire();
	virtual bool isInCooldown();
	virtual void update();

	virtual float getLaunchDistance() const { return 1.0f; }
	virtual int getCooldownFrames() = 0;
	virtual GObject* spawn(float angle, SpaceVect pos) = 0;
protected:
	int cooldownFramesRemaining = 0;
};

class FlandreBigOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline FlandreBigOrbPattern(Player *const player) : SingleBulletFixedIntervalPattern(player) {}

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_big_orb.png"; }
	virtual int powerCost() const { return 5; }

	virtual inline int getCooldownFrames() { return 60; }
	virtual GObject* spawn(float angle, SpaceVect pos);
protected:
	int cooldownFramesRemaining = 0;
};

class FlandreFastOrbPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline FlandreFastOrbPattern(Player *const player) : SingleBulletFixedIntervalPattern(player) {}

	virtual string iconPath() const { return "sprites/fire_patterns/flandre_fast_orb.png"; }
	virtual int powerCost() const { return 1; }

	virtual inline int getCooldownFrames() { return 10; }
	virtual GObject* spawn(float angle, SpaceVect pos);
protected:
	int cooldownFramesRemaining = 0;
};

#endif /* FirePattern_hpp */
