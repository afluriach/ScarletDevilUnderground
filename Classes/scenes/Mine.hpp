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

	Mine();
	inline virtual ~Mine() {}

	inline virtual Color3B getDefaultAmbientLight() const { return toColor3B(Color4F(.4f, .4f, .6f, 1.0f)); }
	virtual GScene* getReplacementScene();
};

#endif /* Mine_hpp */
