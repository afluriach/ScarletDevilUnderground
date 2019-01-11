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

class MultiBulletFixedIntervalPattern : public FirePattern
{
public:
	inline MultiBulletFixedIntervalPattern(Agent *const agent) : FirePattern(agent) {}

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();

	void fire();

	virtual SpaceFloat getLaunchDistance() const { return 1.0; }
	virtual boost::rational<int> getCooldownTime() = 0;
	virtual list<GObject::GeneratorType> spawn(SpaceFloat angle, SpaceVect pos) = 0;
protected:
	boost::rational<int> cooldownTimeRemaining = 0;
};

#endif /* FirePattern_hpp */
