//
//  Launcher.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "Launcher.hpp"

const SpaceFloat Launcher::fireInterval = 0.25;

Launcher::Launcher(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(), physics_params(GType::environment, PhysicsLayers::eyeLevelHeight, args, -1.0))
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
