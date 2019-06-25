//
//  Launcher.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "Launcher.hpp"

const boost::rational<int> Launcher::fireInterval(1,4);

Launcher::Launcher(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(), physics_params(args, -1.0))
{
	auto it = args.find("direction");
	if (it != args.end()) {
		Direction dir = stringToDirection(it->second.asString());
		if (dir != Direction::none)
			setInitialAngle(dirToPhysicsAngle(dir));
	}
}

void Launcher::update()
{
	GObject::update();

	timerDecrement(cooldownTime);

	if (isActive && cooldownTime <= 0) {
		SpaceVect pos = getPos();
		pos += SpaceVect::ray(1.0f, getAngle());

		space->createObject<BulletImpl>(
			Bullet::makeParams(pos, getAngle()),
			bullet_attributes::getDefault(),
			app::getBullet("launcherBullet")
		);
		cooldownTime = fireInterval;
	}
}
