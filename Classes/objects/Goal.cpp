//
//  Goal.cpp
//  Koumachika
//
//  Created by Toni on 12/5/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "Goal.hpp"
#include "GSpace.hpp"
#include "PlayScene.hpp"

Goal::Goal(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject)
{
	setInitialAngle(float_pi / 2.0);
	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Goal created outside of PlayScene!");
	}
}

bool Goal::canInteract()
{
	return !isBlocked;
}

void Goal::interact()
{
	if (isBlocked || space->getIsRunningReplay()) return;

	space->addSceneAction(
		[=]()->void { playScene->triggerSceneCompleted(); }
	);
}

void Goal::activate()
{
	isBlocked = true;
}

void Goal::deactivate()
{
	isBlocked = false;
}
