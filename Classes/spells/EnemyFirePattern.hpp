//
//  EnemyFirePattern.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef EnemyFirePattern_hpp
#define EnemyFirePattern_hpp

#include "FirePattern.hpp"

class IceFairyBulletPattern : public SingleBulletFixedIntervalPattern
{
public:
	inline IceFairyBulletPattern(Agent *const agent) : SingleBulletFixedIntervalPattern(agent) {}

	//not relevant for enemy fire patterns
	virtual string iconPath() const { return ""; }

	virtual boost::rational<int> getCooldownTime() { return 2.0; }
	virtual GObject::GeneratorType spawn(SpaceFloat angle, SpaceVect pos);
};

class Fairy1ABulletPattern : public MultiBulletFixedIntervalPattern
{
public:
	inline Fairy1ABulletPattern(Agent *const agent) : MultiBulletFixedIntervalPattern(agent) {}

	//not relevant for enemy fire patterns
	virtual string iconPath() const { return ""; }

	virtual boost::rational<int> getCooldownTime() { return 1.5; }
	virtual list<GObject::GeneratorType> spawn(SpaceFloat angle, SpaceVect pos);
};

#endif /* EnemyFirePattern_hpp */
