//
//  MarisaSpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "MarisaSpell.hpp"

const string StarlightTyphoon::name = "StarlightTyphoon";
const string StarlightTyphoon::description = "";

const SpaceFloat StarlightTyphoon::width = float_pi / 4.0;
const SpaceFloat StarlightTyphoon::duration = 1.0;
const SpaceFloat StarlightTyphoon::offset = 0.7;

const unsigned int StarlightTyphoon::count = 30;

const vector<string> StarlightTyphoon::colors = {
	"blue",
	"green",
	"grey",
	"indigo",
	"purple",
	"red",
	"yellow"
};

StarlightTyphoon::StarlightTyphoon(GObject* caster) :
	Spell(caster, spell_params{ duration, 0.0 })
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
	SpaceVect pos = caster->getPos() + SpaceVect::ray(offset, angle);
	auto params = Bullet::makeParams(pos, arcPos);

	auto props = make_shared<bullet_properties>();
	*props = *app::getBullet("starBullet");
	props->sprite = "star-" + colors[getSpace()->getRandomInt(0, colors.size() - 1)];
	props->dimensions.x *= space->getRandomFloat(0.7, 1.3);
	props->speed *= space->getRandomFloat(0.5, 1.5);

	space->createObject<BulletImpl>(
		params,
		getCasterAs<Agent>()->getBulletAttributes(props),
		props
	);
}

void StarlightTyphoon::update()
{
	accumulator += shotsPerFrame;

	while (accumulator >= 1) {
		fire();
		accumulator -= 1;
	}
}

void StarlightTyphoon::end()
{
}
