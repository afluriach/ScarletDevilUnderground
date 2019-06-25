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
#include "PlayScene.hpp"

Desk::Desk(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(MapParamsPointUp(), MapRectPhys(-1.0))
{
}

void Desk::interact(Player* p)
{
	space->addSceneAction(
		[=]()->void { space->getSceneAs<PlayScene>()->enterWorldSelect(); }
	);
}
