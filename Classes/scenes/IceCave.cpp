//
//  IceCave.cpp
//  Koumachika
//
//  Created by Toni on 12/21/18.
//
//

#include "Prefix.h"

#include "IceCave.hpp"

const IntVec2 IceCave::roomSize = IntVec2(17,17);

const vector<GScene::MapEntry> IceCave::rooms = {
	{ make_pair("ice_cave/C9", getRoomOffset(roomSize, 0, 2)) },

	{ make_pair("ice_cave/C6", getRoomOffset(roomSize, 0, 3)) },
 	{ make_pair("ice_cave/C0", getRoomOffset(roomSize, 1, 3)) },	
	{ make_pair("ice_cave/C4", getRoomOffset(roomSize, 2, 3)) }
};

IceCave::IceCave() :
PlayScene("IceCave", rooms)
{

}

GScene* IceCave::getReplacementScene()
{
	return Node::ccCreate<IceCave>();
}
