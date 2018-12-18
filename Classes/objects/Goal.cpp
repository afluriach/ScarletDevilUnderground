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
	playScene = space->getSceneAs<PlayScene>();

	if (!playScene) {
		throw runtime_error("Goal created outside of PlayScene!");
	}
}

bool Goal::canInteract()
{
	return true;
}

void Goal::interact()
{
	playScene->triggerSceneCompleted();
}
