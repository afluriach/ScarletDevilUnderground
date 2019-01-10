//
//  Graveyard.hpp
//  Koumachika
//
//  Created by Toni on 1/10/19.
//
//

#ifndef Graveyard_hpp
#define Graveyard_hpp

#include "PlayScene.hpp"

class Graveyard : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Graveyard();
	inline virtual ~Graveyard() {}

	virtual GScene* getReplacementScene();
};

#endif /* Graveyard_hpp */
