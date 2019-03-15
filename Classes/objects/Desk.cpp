//
//  Desk.cpp
//  Koumachika
//
//  Created by Toni on 2/20/19.
//
//

#include "Prefix.h"

#include "Desk.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "PlayScene.hpp"

Desk::Desk(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space,id,args)
{
	setInitialAngle(float_pi / 2.0);
}

void Desk::interact()
{
	space->addSceneAction(
		[=]()->void { space->getSceneAs<PlayScene>()->enterWorldSelect(); }
	);
}
