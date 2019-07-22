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
#include "GSpace.hpp"

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
	auto params = Bullet::makeParams(pos, angle);

	gobject_ref b1 = agent->bulletImplCheckSpawn(params, props);
	gobject_ref b2 = agent->bulletImplCheckSpawn(params, props);

	auto f1 = getParametricFunction(pos, angle, props->speed, 0.0);
	auto f2 = getParametricFunction(pos, angle, props->speed, 0.5);

	agent->makeInitMessage(&GObject::setParametricMove, b1, f1, parametric_type::position);
	agent->makeInitMessage(&GObject::setParametricMove, b2, f2, parametric_type::position);

	cooldown = fireInterval;

	return true;
}
