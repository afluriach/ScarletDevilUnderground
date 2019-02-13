//
//  Graveyard.cpp
//  Koumachika
//
//  Created by Toni on 1/10/19.
//
//

#include "Prefix.h"

#include "Graveyard.hpp"

const IntVec2 Graveyard::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> Graveyard::rooms = {

	{ make_pair("graveyard/G0", getRoomOffset(roomSize, 0, 3)) },
	{ make_pair("graveyard/G1", getRoomOffset(roomSize, 1, 3)) },
	{ make_pair("graveyard/G2", getRoomOffset(roomSize, 2, 3)) },
	{ make_pair("graveyard/G3", getRoomOffset(roomSize, 3, 3)) },

	{ make_pair("graveyard/G4", getRoomOffset(roomSize, 0, 2)) },
	{ make_pair("graveyard/G5", getRoomOffset(roomSize, 1, 2)) },
	{ make_pair("graveyard/G6", getRoomOffset(roomSize, 2, 2)) },
	{ make_pair("graveyard/G7", getRoomOffset(roomSize, 3, 2)) },

	{ make_pair("graveyard/G8", getRoomOffset(roomSize, 0, 1)) },
	{ make_pair("graveyard/G9", getRoomOffset(roomSize, 1, 1)) },
	{ make_pair("graveyard/G10", getRoomOffset(roomSize, 2, 1)) },
	{ make_pair("graveyard/G11", getRoomOffset(roomSize, 3, 1)) },

	{ make_pair("graveyard/G12", getRoomOffset(roomSize, 0, 0)) },
	{ make_pair("graveyard/G13", getRoomOffset(roomSize, 1, 0)) },
	{ make_pair("graveyard/G14", getRoomOffset(roomSize, 2, 0)) },
	{ make_pair("graveyard/G15", getRoomOffset(roomSize, 3, 0)) },
};

Graveyard::Graveyard() :
PlayScene("Graveyard", rooms)
{
}

GScene* Graveyard::getReplacementScene()
{
	return Node::ccCreate<Graveyard>();
}
