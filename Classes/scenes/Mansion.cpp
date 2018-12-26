//
//  Mine.cpp
//  Koumachika
//
//  Created by Toni on 12/26/18.
//
//

#include "Prefix.h"

#include "Mansion.hpp"

const IntVec2 Mansion::roomSize = IntVec2(17,9);

const vector<GScene::MapEntry> Mansion::rooms = {
	{ make_pair("mansion/M5", getRoomOffset(roomSize, 0, 0)) },
	{ make_pair("mansion/M4", getRoomOffset(roomSize, 0, 1)) },
	{ make_pair("mansion/M3", getRoomOffset(roomSize, 0, 2)) },

	{ make_pair("mansion/M1", getRoomOffset(roomSize, 1, 1)) },
	{ make_pair("mansion/M1", getRoomOffset(roomSize, 2, 1)) },
	{ make_pair("mansion/M1", getRoomOffset(roomSize, 3, 1)) },

	{ make_pair("mansion/M2", getRoomOffset(roomSize, 4, 1)) }
};

Mansion::Mansion() :
PlayScene("Mansion", rooms)
{

}

GScene* Mansion::getReplacementScene()
{
	return Node::ccCreate<Mansion>();
}
