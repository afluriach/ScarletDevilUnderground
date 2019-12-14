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

class StarbowBreak : public FirePattern
{
public:
	static constexpr int anglesCount = 5;

	static const float baseCost;
	static const float baseFireInterval;
	static const array<float, anglesCount> angleIntervalScales;

	static const double angleVariation;
	static const double angleStep;

	static const double launchDist;
	static const array<double, anglesCount> radiusScales;

	StarbowBreak(Agent *const agent);

	bullet_properties generateProps(int angle);
	bool spawnBullet(int angle, bool left);

	inline virtual string iconPath() const { return "sprites/ui/starbow_break.png"; }
	virtual void update();
protected:
	virtual bool fire();

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
	static const SpaceFloat fireInterval;

	Catadioptric(Agent *const agent);

	inline virtual void update() { timerDecrement(cooldown); }
	inline virtual string iconPath() const { return "sprites/ui/catadioptric.png"; }
	inline virtual float getCost() const { return 20.0f; }
protected:
	virtual bool fire();
	bool spawnTail(SpaceFloat angleOffset);

	SpaceFloat cooldown = 0.0;
};

#endif /* PlayerFirePattern_hpp */
