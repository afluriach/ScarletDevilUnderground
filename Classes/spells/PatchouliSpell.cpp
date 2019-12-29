//
//  PatchouliSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Bullet.hpp"
#include "PatchouliSpell.hpp"

const string FireStarburst::name = "FireStarburst";
const string FireStarburst::description = "";

FireStarburst::FireStarburst(GObject* caster) : 
	Spell(caster, spell_params{ -1.0, 0.5 })
{
	bulletProps = make_shared<bullet_properties>();
	*bulletProps = *app::getBullet("fireBullet");
	bulletProps->speed = bulletSpeed;
}

void FireStarburst::update()
{
	for_irange(i, 0, 8)
	{
		SpaceFloat angle = float_pi * i / 4.0;
		SpaceVect crntPos = SpaceVect::ray(1, angle);

		getCasterAs<Agent>()->launchBullet(
			bulletProps,
			crntPos,
			angle
		);
	}
}

const string FlameFence::name = "FlameFence";
const string FlameFence::description = "";

FlameFence::FlameFence(GObject* caster) :
	Spell(caster, spell_params{ -1.0, 0 })
{
	bulletProps = make_shared<bullet_properties>();
	*bulletProps = *app::getBullet("fireBullet");
	bulletProps->speed = 0.0;
	bulletProps->directionalLaunch = false;
}

void FlameFence::init()
{
	for (int y = -10; y < 10; y += 2)
	{
		SpaceVect rowSkew(y % 2 ? 0.5 : 0, 0);
		for (int x = -10; x < 10; x += 2)
		{
			SpaceVect pos = SpaceVect(x, y) + rowSkew;

			bullets.push_back(getCasterAs<Agent>()->spawnBullet(
				bulletProps,
				pos,
				SpaceVect::zero,
				0.0,
				0.0
			));
		}
	}
}

void FlameFence::end()
{
	for (gobject_ref bullet : bullets) {
		if (bullet.isValid())
			getSpace()->removeObject(bullet);
	}
}
