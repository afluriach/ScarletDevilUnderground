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
	{make_pair("graveyard/G0", getRoomOffset(roomSize, 0, 0))},
};

Graveyard::Graveyard() :
PlayScene("Graveyard", rooms)
{
}

GScene* Graveyard::getReplacementScene()
{
	return Node::ccCreate<Graveyard>();
}
