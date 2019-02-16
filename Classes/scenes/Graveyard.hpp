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

class Graveyard1 : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Graveyard1();
	inline virtual ~Graveyard1() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.3f, .45f, .45f, 1.0f); }

	virtual GScene* getReplacementScene();
};

class Graveyard2 : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Graveyard2();
	inline virtual ~Graveyard2() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.225f, .333f, .333f, 1.0f); }

	virtual GScene* getReplacementScene();
};


#endif /* Graveyard_hpp */
