//
//  Launcher.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "App.h"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "Launcher.hpp"
#include "util.h"

const boost::rational<int> Launcher::fireInterval(1,4);

Launcher::Launcher(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	RegisterUpdate<Launcher>(this)
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
	timerDecrement(cooldownTime);

	if (isActive && cooldownTime <= 0) {
		SpaceVect pos = getPos();
		pos += SpaceVect::ray(1.0f, getAngle());

		space->createObject(
			GObject::make_object_factory<LauncherBullet>(getAngle(), pos)
		);
		cooldownTime = fireInterval;
	}
}