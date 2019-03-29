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

#define GET_MAPS inline virtual MapFragmentsList getMapFragmentsList() const { return maps; }

class Graveyard1 : public PlayScene
{
public:
	Graveyard1();
	inline virtual ~Graveyard1() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.3f, .45f, .45f, 1.0f); }

	virtual GScene* getReplacementScene();
	inline virtual ChamberID getCurrentLevel() const { return ChamberID::graveyard1; }
	inline virtual ChamberID getNextLevel() const { return ChamberID::graveyard2; }
};

class Graveyard2 : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Graveyard2();
	inline virtual ~Graveyard2() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.3f, .45f, .45f, 1.0f); }

	virtual GScene* getReplacementScene();
	inline virtual ChamberID getCurrentLevel() const { return ChamberID::graveyard2; }
	inline virtual ChamberID getNextLevel() const { return ChamberID::graveyard3; }
};

class Graveyard3 : public PlayScene
{
public:
	static const MapFragmentsList maps;

	Graveyard3();
	inline virtual ~Graveyard3() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.225f, .333f, .333f, 1.0f); }

	virtual GScene* getReplacementScene();
	inline virtual ChamberID getCurrentLevel() const { return ChamberID::graveyard3; }
	inline virtual ChamberID getNextLevel() const { return ChamberID::graveyard4; }
	GET_MAPS;
};

class Graveyard4 : public PlayScene
{
public:
	static const IntVec2 roomSize;
	static const vector<MapEntry> rooms;

	Graveyard4();
	inline virtual ~Graveyard4() {}

	inline virtual Color4F getDefaultAmbientLight() const { return Color4F(.225f, .333f, .333f, 1.0f); }

	virtual GScene* getReplacementScene();
	inline virtual ChamberID getCurrentLevel() const { return ChamberID::graveyard4; }
};

#endif /* Graveyard_hpp */
