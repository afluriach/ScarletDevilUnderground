//
//  OverworldScene.cpp
//  Koumachika
//
//  Created by Toni on 2/26/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "Door.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "OverworldScene.hpp"
#include "Player.hpp"

OverworldScene::OverworldScene(string mapName, string start) :
PlayScene(mapName),
mapName(mapName),
start(start)
{
	multiInit.insertWithOrder(
		bind(&OverworldScene::initHUD, this),
		to_int(initOrder::postInitHUD)
	);
	multiInit.insertWithOrder(
		bind(&OverworldScene::loadPlayer, this),
		to_int(initOrder::postLoadObjects)
	);
}

void OverworldScene::initHUD()
{
	setIsOverworld(true);
	hud->setMansionMode(true);

	gspace->addObjectAction([=]()->void {
		gspace->setSuppressAction(true);
	});
}

void OverworldScene::loadPlayer()
{
	Door* door = gspace->getObjectAs<Door>(start);
	SpaceVect player_start = gspace->getWaypoint(start);

	if (door) {
		SpaceVect pos = door->getEntryPosition();
		Direction d = door->getDoorDirection();

		gspace->createObject(GObject::make_object_factory<FlandrePC>(pos, d));
	}
	else if(!player_start.isZero())
	{
		gspace->createObject(GObject::make_object_factory<FlandrePC>(player_start, Direction::up));
	}
	else
	{
		log("Overworld scene %s, unknown player start!", mapName.c_str());
	}
}

GScene* OverworldScene::getReplacementScene()
{
	return Node::ccCreate<OverworldScene>(mapName, start);
}
