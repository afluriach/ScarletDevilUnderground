//
//  EnemyFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "EnemyFirePattern.hpp"

const SpaceFloat ReimuWavePattern::omega = float_pi * 2.0;
const SpaceFloat ReimuWavePattern::amplitude = 2.0;
const SpaceFloat ReimuWavePattern::fireInterval = 0.75;

SpaceVect ReimuWavePattern::parametric_move(
	SpaceFloat t,
	SpaceFloat firingAngle,
	SpaceFloat phaseAngleStart,
	SpaceFloat speed
) {
	SpaceVect d1 = SpaceVect::ray(t * speed, firingAngle);
	SpaceVect d2 = SpaceVect::ray(amplitude, firingAngle + float_pi * 0.5)*cos((t + phaseAngleStart)*omega);

	return d1 + d2;
}

parametric_space_function ReimuWavePattern::getParametricFunction(SpaceVect origin, SpaceFloat angle, SpaceFloat speed, SpaceFloat tOffset)
{
	return ai::parametricMoveTranslate(
		bind(&parametric_move, placeholders::_1, angle, tOffset, speed),
		origin,
		//In this case, we are already capturing our parametric variable start offset.
		0.0
	);
}

ReimuWavePattern::ReimuWavePattern(Agent *const agent) :
	FirePattern(agent)
{
	props = app::getBullet("reimuBullet1");
}

bool ReimuWavePattern::fire()
{
	if (cooldown > 0.0) return false;

	SpaceVect pos = agent->getPos();
	SpaceFloat angle = agent->getAngle();

	auto f1 = getParametricFunction(pos, angle, props->speed, 0.0);
	auto f2 = getParametricFunction(pos, angle, props->speed, 0.5);

	gobject_ref b1 = agent->parametricBullet(props, SpaceVect::zero, f1, angle);
	gobject_ref b2 = agent->parametricBullet(props, SpaceVect::zero, f2, angle);

	cooldown = fireInterval;

	return true;
}
