//
//  OverworldScene.hpp
//  Koumachika
//
//  Created by Toni on 2/26/19.
//
//

#ifndef OverworldScene_hpp
#define OverworldScene_hpp

#include "PlayScene.hpp"

class OverworldScene : public PlayScene
{
public:
	OverworldScene(string mapName, string start);
	inline virtual ~OverworldScene() {}

	void initHUD();
	void loadPlayer();

	virtual GScene* getReplacementScene();
protected:
	string mapName;
	string start;
};

#endif /* OverworldScene_hpp */
