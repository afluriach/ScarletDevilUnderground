//
//  Mine.hpp
//  Koumachika
//
//  Created by Toni on 12/11/18.
//
//

#ifndef Mine_hpp
#define Mine_hpp

#include "PlayScene.hpp"

class Mine : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	static const IntVec2 getRoomOffset(int roomGridX, int roomGridY);

	CREATE_FUNC(Mine)

	Mine();
	inline virtual ~Mine() {}
};

#endif /* Mine_hpp */
