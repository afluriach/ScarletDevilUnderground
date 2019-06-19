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
#include "GState.hpp"
#include "HUD.hpp"
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

	if (mapName == "overworld/forest" || mapName == "overworld/forest_lake") {
		hud->setObjectiveCounter("sprites/mushroom.png", App::getCrntState()->mushroomCount);
	}
}

void OverworldScene::loadPlayer()
{
	Door* door = gspace->getObjectAs<Door>(start);
	SpaceVect player_start = gspace->getWaypoint(start);

	if (door) {
		SpaceVect pos = door->getEntryPosition();
		Direction d = door->getEntryDirection();

		gspace->createObject<FlandrePC>(pos, d);
	}
	else if(!player_start.isZero())
	{
		gspace->createObject<FlandrePC>(player_start, Direction::up);
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
