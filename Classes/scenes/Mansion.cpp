//
//  Mine.cpp
//  Koumachika
//
//  Created by Toni on 12/26/18.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "macros.h"
#include "Mansion.hpp"

const IntVec2 Mansion::roomSize = IntVec2(17,9);

const vector<GScene::MapEntry> Mansion::rooms = {
	{ make_pair("mansion/M5", getRoomOffset(roomSize, 1, 0)) },

	{ make_pair("mansion/M0", getRoomOffset(roomSize, 0, 1)) },
	{ make_pair("mansion/M4", getRoomOffset(roomSize, 1, 1)) },
	{ make_pair("mansion/M1", getRoomOffset(roomSize, 2, 1)) },
	{ make_pair("mansion/M2", getRoomOffset(roomSize, 3, 1)) },

	{ make_pair("mansion/M3", getRoomOffset(roomSize, 1, 2)) },
};

Mansion::Mansion() :
PlayScene("Mansion", rooms)
{
	multiInit.insertWithOrder(
		bind(&Mansion::initHUD, this),
		to_int(initOrder::postInitHUD)
	);
}

void Mansion::initHUD()
{
	setIsOverworld(true);
	hud->setMansionMode(true);

	gspace->addObjectAction([=]()->void {
		gspace->setSuppressAction(true);
	});
}

GScene* Mansion::getReplacementScene()
{
	return Node::ccCreate<Mansion>();
}
