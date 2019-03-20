//
//  Graveyard.cpp
//  Koumachika
//
//  Created by Toni on 1/10/19.
//
//

#include "Prefix.h"

#include "Graveyard.hpp"

Graveyard0::Graveyard0() :
	PlayScene("G0")
{
}

GScene* Graveyard0::getReplacementScene()
{
	return Node::ccCreate<Graveyard0>();
}

const IntVec2 Graveyard1::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> Graveyard1::rooms = {
	{ make_pair("graveyard1/G0", getRoomOffset(roomSize, 0, 3)) },
	{ make_pair("graveyard1/G1", getRoomOffset(roomSize, 1, 3)) },
	{ make_pair("graveyard1/G2", getRoomOffset(roomSize, 2, 3)) },
	{ make_pair("graveyard1/G3", getRoomOffset(roomSize, 3, 3)) },

	{ make_pair("graveyard1/G4", getRoomOffset(roomSize, 0, 2)) },
	{ make_pair("graveyard1/G5", getRoomOffset(roomSize, 1, 2)) },
	{ make_pair("graveyard1/G6", getRoomOffset(roomSize, 2, 2)) },
	{ make_pair("graveyard1/G7", getRoomOffset(roomSize, 3, 2)) },

	{ make_pair("graveyard1/G8", getRoomOffset(roomSize, 0, 1)) },
	{ make_pair("graveyard1/G9", getRoomOffset(roomSize, 1, 1)) },
	{ make_pair("graveyard1/G10", getRoomOffset(roomSize, 2, 1)) },
	{ make_pair("graveyard1/G11", getRoomOffset(roomSize, 3, 1)) },

	{ make_pair("graveyard1/G12", getRoomOffset(roomSize, 0, 0)) },
	{ make_pair("graveyard1/G13", getRoomOffset(roomSize, 1, 0)) },
	{ make_pair("graveyard1/G14", getRoomOffset(roomSize, 2, 0)) },
	{ make_pair("graveyard1/G15", getRoomOffset(roomSize, 3, 0)) },
};

Graveyard1::Graveyard1() :
PlayScene("Graveyard1", rooms)
{
}

GScene* Graveyard1::getReplacementScene()
{
	return Node::ccCreate<Graveyard1>();
}

const IntVec2 Graveyard4::roomSize = IntVec2(17, 17);

const vector<GScene::MapEntry> Graveyard4::rooms = {
	{ make_pair("graveyard4/G0", getRoomOffset(roomSize, 0, 3)) },
	{ make_pair("graveyard4/G1", getRoomOffset(roomSize, 1, 3)) },
	{ make_pair("graveyard4/G2", getRoomOffset(roomSize, 2, 3)) },
	{ make_pair("graveyard4/G3", getRoomOffset(roomSize, 3, 3)) },

	{ make_pair("graveyard4/G4", getRoomOffset(roomSize, 0, 2)) },
	{ make_pair("graveyard4/G5", getRoomOffset(roomSize, 1, 2)) },
	{ make_pair("graveyard4/G6", getRoomOffset(roomSize, 2, 2)) },
	{ make_pair("graveyard4/G7", getRoomOffset(roomSize, 3, 2)) },

	{ make_pair("graveyard4/G8", getRoomOffset(roomSize, 0, 1)) },
	{ make_pair("graveyard4/G9", getRoomOffset(roomSize, 1, 1)) },
	{ make_pair("graveyard4/G10", getRoomOffset(roomSize, 2, 1)) },
	{ make_pair("graveyard4/G11", getRoomOffset(roomSize, 3, 1)) },

	{ make_pair("graveyard4/G12", getRoomOffset(roomSize, 0, 0)) },
	{ make_pair("graveyard4/G13", getRoomOffset(roomSize, 1, 0)) },
	{ make_pair("graveyard4/G14", getRoomOffset(roomSize, 2, 0)) },
	{ make_pair("graveyard4/G15", getRoomOffset(roomSize, 3, 0)) },
};

Graveyard4::Graveyard4() :
	PlayScene("Graveyard4", rooms)
{
}

GScene* Graveyard4::getReplacementScene()
{
	return Node::ccCreate<Graveyard4>();
	return Node::ccCreate<Graveyard4>();
}
