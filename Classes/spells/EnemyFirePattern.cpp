//
//  EnemyFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "EnemyBullet.hpp"
#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"

GObject::GeneratorType IceFairyBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<IceFairyBullet>(angle, pos);
}

list<GObject::GeneratorType> Fairy1ABulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	list<GObject::GeneratorType> result;

	result.push_back(GObject::make_object_factory<Fairy1Bullet>(
		angle - float_pi / 6.0,
		pos + SpaceVect::ray(getLaunchDistance(), angle - float_pi / 6.0)
	));

	result.push_back(GObject::make_object_factory<Fairy1Bullet>(
		angle,
		pos + SpaceVect::ray(getLaunchDistance(), angle)
	));

	result.push_back(GObject::make_object_factory<Fairy1Bullet>(
		angle + float_pi / 6.0,
		pos + SpaceVect::ray(getLaunchDistance(), angle + float_pi / 6.0)
	));

	return result;
}
