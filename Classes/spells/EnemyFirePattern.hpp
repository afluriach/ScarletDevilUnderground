//
//  EnemyFirePattern.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef EnemyFirePattern_hpp
#define EnemyFirePattern_hpp

#include "EnemyBullet.hpp"
#include "FirePattern.hpp"

class IceFairyBulletPattern : public SingleBulletFixedIntervalPattern, public FirePatternImpl<IceFairyBullet>
{
public:
	IceFairyBulletPattern(Agent *const agent);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }

	inline virtual float getCooldownTime() { return 2.0f; }
};

class Fairy1BulletPattern : public MultiBulletSpreadPattern, public FirePatternImpl<Fairy1Bullet>
{
public:
	Fairy1BulletPattern(
		Agent *const agent,
		float fireInterval,
		SpaceFloat sideAngleSpread,
		int bulletCount
	);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }

	inline virtual float getCooldownTime() { return 1.5f; }
};

class GreenFairyBulletPattern : public RadiusPattern, public FirePatternImpl<GreenFairyBullet>
{
public:
	GreenFairyBulletPattern(
		Agent *const agent,
		float fireInterval,
		int bulletCount
	);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }

	inline virtual float getCooldownTime() { return 1.5f; }
};

class RumiaBurstPattern : public BurstPattern, public FirePatternImpl<RumiaBullet>
{
public:
	RumiaBurstPattern(Agent *const agent);

	//not relevant for enemy fire patterns
	inline virtual string iconPath() const { return ""; }
};

#endif /* EnemyFirePattern_hpp */
