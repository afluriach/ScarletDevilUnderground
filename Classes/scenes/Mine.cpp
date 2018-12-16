//
//  Mine.cpp
//  Koumachika
//
//  Created by Toni on 12/11/18.
//
//

#include "Prefix.h"

#include "Mine.hpp"

const IntVec2 Mine::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> Mine::rooms = {
	{make_pair("mine/M9", getRoomOffset(0, 0))},
	{make_pair("mine/M9", getRoomOffset(2, 0))},

	{make_pair("mine/M2", getRoomOffset(0, 1))},
	{make_pair("mine/M7", getRoomOffset(1, 1))},
	{make_pair("mine/M1", getRoomOffset(2, 1))},
	{make_pair("mine/M4", getRoomOffset(3, 1))},

	{make_pair("mine/M5", getRoomOffset(0, 2))},
	{make_pair("mine/M8", getRoomOffset(2, 2))},

	{make_pair("mine/M6", getRoomOffset(0, 3))},
	{make_pair("mine/M0", getRoomOffset(1, 3))},
	{make_pair("mine/M3", getRoomOffset(2, 3))},
	{make_pair("mine/M4", getRoomOffset(3, 3))}
};

const IntVec2 Mine::getRoomOffset(int roomGridX, int roomGridY)
{
	return IntVec2(roomGridX * roomSize.first, roomGridY * roomSize.second);
}

Mine::Mine() :
PlayScene("Mine", rooms)
{

}

GScene* Mine::getReplacementScene()
{
	return Node::ccCreate<Mine>();
}
