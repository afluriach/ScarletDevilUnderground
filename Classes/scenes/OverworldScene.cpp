//
//  OverworldScene.cpp
//  Koumachika
//
//  Created by Toni on 2/26/19.
//
//

#include "Prefix.h"

#include "Door.hpp"
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
}

void OverworldScene::initHUD()
{
	setIsOverworld(true);
	hud->setMansionMode(true);

	gspace->addObjectAction([=]()->void {
		gspace->setSuppressAction(true);
	});

	if ( (mapName == "overworld/forest" || mapName == "overworld/forest_lake") && App::getCrntState()->hasAttribute("mushroomCount") ) {
		hud->setObjectiveCounter("sprites/mushroom.png", App::getCrntState()->getAttribute("mushroomCount"));
	}
}

GScene* OverworldScene::getReplacementScene()
{
	return Node::ccCreate<OverworldScene>(mapName, start);
}
