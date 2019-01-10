//
//  Forest.hpp
//  Koumachika
//
//  Created by Toni on 1/9/198.
//
//

#ifndef Forest_hpp
#define Forest_hpp

#include "PlayScene.hpp"

class Forest : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Forest();
	inline virtual ~Forest() {}

	virtual GScene* getReplacementScene();
};

#endif /* Forest_hpp */
