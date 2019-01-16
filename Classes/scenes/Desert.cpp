//
//  Desert.cpp
//  Koumachika
//
//  Created by Toni on 12/11/18.
//
//

#include "Prefix.h"

#include "Desert.hpp"

const IntVec2 Desert::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> Desert::rooms = {
	{make_pair("desert/D2", getRoomOffset(roomSize, 0, 0))},
	{make_pair("desert/D0", getRoomOffset(roomSize, 1, 0))},
	{make_pair("desert/D1", getRoomOffset(roomSize, 2, 0))},
	{make_pair("desert/D3", getRoomOffset(roomSize, 3, 0))},
	{make_pair("desert/D4", getRoomOffset(roomSize, 4, 0))}
};

Desert::Desert() :
PlayScene("Desert", rooms)
{
	ambientLight = Color4F(.7f, .7f, .3f,1.0f);
}

GScene* Desert::getReplacementScene()
{
	return Node::ccCreate<Desert>();
}
