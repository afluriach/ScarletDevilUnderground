//
//  MarisaSpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
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

	shotsPerFrame = count / duration * App::secondsPerFrame;
}

void StarlightTyphoon::init()
{
}

void StarlightTyphoon::fire()
{
	//The angle variation, will be added to the base direction.
	SpaceFloat arcPos = caster->space->getRandomFloat(-width, width) + caster->getAngle();
	SpaceFloat crntSpeed = caster->space->getRandomFloat(speed*0.5, speed*1.5);
	SpaceFloat crntRadius = caster->space->getRandomFloat(radius*0.7, radius*1.3);

	SpaceVect pos = caster->getPos() + SpaceVect::ray(offset, angle);

	caster->space->createObject(GObject::make_object_factory<StarBullet>(
		pos,
		arcPos,
		dynamic_cast<Agent*>(caster),
		crntSpeed,
		radius,
		StarBullet::colors[caster->space->getRandomInt(0, StarBullet::colors.size() - 1)]
	));
}

void StarlightTyphoon::update()
{
	accumulator += shotsPerFrame;
	elapsed += App::secondsPerFrame;

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
