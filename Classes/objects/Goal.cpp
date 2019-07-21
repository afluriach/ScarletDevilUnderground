//
//  Goal.cpp
//  Koumachika
//
//  Created by Toni on 12/5/18.
//
//

#include "Prefix.h"

#include "Goal.hpp"
#include "GSpace.hpp"
#include "PlayScene.hpp"
#include "value_map.hpp"

Goal::Goal(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), MapRectPhys(-1.0))
{
	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Goal created outside of PlayScene!");
	}
}

void Goal::update()
{
	GObject::update();

	timerDecrement(audioTimer);
	if (audioTimer <= 0.0 && !isBlocked)
	{
		playSoundSpatial("sfx/shot.wav", 0.5f);
		audioTimer = 0.15;
	}
}

bool Goal::canInteract(Player* p)
{
	return !isBlocked;
}

void Goal::interact(Player* p)
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
