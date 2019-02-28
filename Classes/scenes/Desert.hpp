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

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.7f, .7f, .3f, 1.0f); }

	virtual GScene* getReplacementScene();
	inline virtual ChamberID getCurrentLevel() const { return ChamberID::desert1; }
};

#endif /* Desert_hpp */
