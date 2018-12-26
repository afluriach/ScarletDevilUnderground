//
//  Mansion.hpp
//  Koumachika
//
//  Created by Toni on 12/26/18.
//
//

#ifndef Mansion_hpp
#define Mansion_hpp

#include "PlayScene.hpp"

class Mansion : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Mansion();
	inline virtual ~Mansion() {}

	virtual GScene* getReplacementScene();
};

#endif /* Mine_hpp */
