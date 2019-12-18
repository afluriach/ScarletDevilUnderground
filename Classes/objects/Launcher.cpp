//
//  Launcher.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "GSpace.hpp"
#include "Launcher.hpp"

const boost::rational<int> Launcher::fireInterval(1,4);

Launcher::Launcher(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(), physics_params(GType::environment, eyeLevelHeightLayers, args, -1.0))
{
}

void Launcher::update()
{
	GObject::update();

	timerDecrement(cooldownTime);

	if (isActive && cooldownTime <= 0) {
		SpaceVect pos = getPos();
		pos += SpaceVect::ray(1.0f, getAngle());

		space->createBullet(
			Bullet::makeParams(pos, getAngle()),
			bullet_attributes::getDefault(),
			app::getBullet("launcherBullet")
		);
		cooldownTime = fireInterval;
	}
}
