//
//  MarisaSpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "MarisaSpell.hpp"

const string StarlightTyphoon::name = "StarlightTyphoon";
const string StarlightTyphoon::description = "";
const float StarlightTyphoon::cost = 0.0f;

const SpaceFloat StarlightTyphoon::speed = 6.0;
const SpaceFloat StarlightTyphoon::width = float_pi / 4.0;
const SpaceFloat StarlightTyphoon::radius = 0.2;
const SpaceFloat StarlightTyphoon::duration = 1.0;
const SpaceFloat StarlightTyphoon::offset = 0.7;

const unsigned int StarlightTyphoon::count = 30;

StarlightTyphoon::StarlightTyphoon(GObject* caster) :
	Spell(caster)
{
	angle = caster->getAngle();

	shotsPerFrame = count / duration * app::params.secondsPerFrame;
}

void StarlightTyphoon::init()
{
}

void StarlightTyphoon::fire()
{
	GSpace* space = getSpace();

	//The angle variation, will be added to the base direction.
	SpaceFloat arcPos = space->getRandomFloat(-width, width) + caster->getAngle();
	SpaceFloat crntSpeed = space->getRandomFloat(speed*0.5, speed*1.5);
	SpaceFloat crntRadius = space->getRandomFloat(radius*0.7, radius*1.3);

	SpaceVect pos = caster->getPos() + SpaceVect::ray(offset, angle);
	auto params = Bullet::makeParams(pos, arcPos);

	space->createObject<StarBullet>(
		params,
		bullet_attributes::getDefault(),
		StarBullet::colors[getSpace()->getRandomInt(0, StarBullet::colors.size() - 1)]
	);
}

void StarlightTyphoon::update()
{
	accumulator += shotsPerFrame;
	timerIncrement(elapsed);

	while (accumulator >= 1) {
		fire();
		accumulator -= 1;
	}

	if (elapsed > duration) {
		caster->stopSpell();
	}
}

void StarlightTyphoon::end()
{
}
