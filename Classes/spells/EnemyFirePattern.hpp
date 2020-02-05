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

class ReimuWavePattern : public FirePattern
{
public:
	static const SpaceFloat omega;
	static const SpaceFloat amplitude;
	static const SpaceFloat fireInterval;

	static SpaceVect parametric_move(
		SpaceFloat t,
		SpaceFloat angle,
		SpaceFloat phaseAngleStart,
		SpaceFloat speed
	);

	static parametric_space_function getParametricFunction(
		SpaceVect origin,
		SpaceFloat angle,
		SpaceFloat speed,
		SpaceFloat tOffset
	);

	ReimuWavePattern(Agent *const agent);
	
	virtual bool fire();
	inline virtual void update() { timerDecrement(cooldown); }
protected:
	local_shared_ptr<bullet_properties> props;
	SpaceFloat cooldown = 0.0;
};

#endif /* EnemyFirePattern_hpp */
