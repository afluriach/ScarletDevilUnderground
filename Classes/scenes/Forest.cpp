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
	{make_pair("forest/F0", IntVec2(53,16))},
	{make_pair("forest/F1", IntVec2(70,16))},
	{make_pair("forest/F2", IntVec2(87,16))},
	{make_pair("forest/F3", IntVec2(0,1))},
	{make_pair("forest/F4", IntVec2(39,39))},
	{make_pair("forest/F5", IntVec2(0,38))},
	{make_pair("forest/F6", IntVec2(11,77))},
	{make_pair("forest/F7", IntVec2(11,98))},
	{make_pair("forest/F8", IntVec2(44,77))},
};

Forest::Forest() :
PlayScene("Forest", rooms)
{

}

GScene* Forest::getReplacementScene()
{
	return Node::ccCreate<Forest>();
}
