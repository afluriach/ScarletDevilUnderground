//
//  IceCave.hpp
//  Koumachika
//
//  Created by Toni on 12/21/18.
//
//

#ifndef IceCave_hpp
#define IceCave_hpp

#include "PlayScene.hpp"

class IceCave : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	IceCave();
	inline virtual ~IceCave() {}

	virtual GScene* getReplacementScene();
};

#endif /* IceCave_hpp */
