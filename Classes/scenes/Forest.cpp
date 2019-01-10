//
//  Forest.cpp
//  Koumachika
//
//  Created by Toni on 1/9/19.
//
//

#include "Prefix.h"

#include "Forest.hpp"

const IntVec2 Forest::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> Forest::rooms = {
	{make_pair("forest/F0", getRoomOffset(roomSize, 0, 0))},
	{make_pair("forest/F1", getRoomOffset(roomSize, 1, 0))},
	{make_pair("forest/F2", getRoomOffset(roomSize, 2, 0))},
};

Forest::Forest() :
PlayScene("Forest", rooms)
{

}

GScene* Forest::getReplacementScene()
{
	return Node::ccCreate<Forest>();
}
