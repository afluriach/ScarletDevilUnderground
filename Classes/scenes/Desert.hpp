//
//  Desert.hpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#ifndef Desert_hpp
#define Desert_hpp

#include "PlayScene.hpp"

class Desert : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Desert();
	inline virtual ~Desert() {}

	virtual GScene* getReplacementScene();
};

#endif /* Desert_hpp */
