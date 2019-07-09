//
//  SakuyaSpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "SakuyaSpell.hpp"

const string IllusionDial::name = "IllusionDial";
const string IllusionDial::description = "";

const int IllusionDial::count = 16;

const float IllusionDial::radius = 2.5f;
const float IllusionDial::arc_start = 0.0f;
const float IllusionDial::arc_end = float_pi * 2.0f;
const float IllusionDial::arc_width = arc_end - arc_start;
const float IllusionDial::arc_spacing = arc_width / (count - 1);
const float IllusionDial::angular_speed = float_pi * 2.0f / 3.0f;

const float IllusionDial::max_angle_margin = float_pi / 12.0f;
const float IllusionDial::min_fire_interval = 1.0f / 3.0f;

IllusionDial::IllusionDial(GObject* caster) :
	Spell(caster),
	bullets(count),
	launch_flags(count, false)
{}

void IllusionDial::init()
{
	target = caster->space->getPlayer();
	props = app::getBullet("illusionDialDagger");

	//refactor utility function for this
	//getFloatOrDefault(args, "radius", radius);

	for_irange(i, 0, count)
	{
		auto params = Bullet::makeParams(
			caster->getPos() + SpaceVect::ray(radius, arc_start + i * arc_spacing),
			0.0,
			SpaceVect::zero,
			i % 2 ? angular_speed : -angular_speed
		);

		bullets[i] = getSpace()->createObject<BulletImpl>(
			params,
			bullet_attributes::getDefault(),
			props
		);
	}
}

void IllusionDial::update()
{
	++framesSinceLastFire;

	if (framesSinceLastFire*app::params.secondsPerFrame >= min_fire_interval)
	{

		int best = -1;
		SpaceFloat best_angle = float_pi;
		bool allBulletsConsumed = true;

		for_irange(i, 0, count)
		{
			if (bullets[i].isValid() && !launch_flags[i]) {
				allBulletsConsumed = false;

				SpaceFloat crnt =  ai::viewAngleToTarget( bullets[i].get(), target);

				if (!isinf(crnt) && abs(crnt) < best_angle)
					best = i;
				best_angle = abs(crnt);
			}
		}

		if (best != -1 && best_angle < max_angle_margin)
		{
			bullets[best].get()->launchAtTarget(target);
			launch_flags[best] = true;
			framesSinceLastFire = 0;
		}
		else if (allBulletsConsumed)
		{
			//Deactivate spell if all bullets are consumed.
			active = false;
		}
	}
}

void IllusionDial::end()
{
	for (auto ref : bullets) {
		if (ref.isValid()) {
			getSpace()->removeObject(ref);
		}
	}
}
