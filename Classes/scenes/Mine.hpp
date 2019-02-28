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

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.4f, .4f, .6f, 1.0f); }
	virtual GScene* getReplacementScene();
	inline virtual ChamberID getCurrentLevel() const { return ChamberID::mine1; }
};

#endif /* Mine_hpp */
