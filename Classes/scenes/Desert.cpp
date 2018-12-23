//
//  Mine.cpp
//  Koumachika
//
//  Created by Toni on 12/11/18.
//
//

#include "Prefix.h"

#include "Desert.hpp"

const IntVec2 Desert::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> Desert::rooms = {
	{make_pair("desert/D0", getRoomOffset(roomSize, 0, 0))}
};

Desert::Desert() :
PlayScene("Desert", rooms)
{

}

GScene* Desert::getReplacementScene()
{
	return Node::ccCreate<Desert>();
}