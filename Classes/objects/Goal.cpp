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
#include "PlayScene.hpp"

Goal::Goal(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject)
{
}

bool Goal::canInteract()
{
	return true;
}

void Goal::interact()
{
	app->playScene->triggerSceneCompleted();
}
