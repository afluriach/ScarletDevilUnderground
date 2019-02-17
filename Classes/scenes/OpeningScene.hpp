//
//  OpeningScene.hpp
//  Koumachika
//
//  Created by Toni on 2/17/19.
//
//

#ifndef OpeningScene_hpp
#define OpeningScene_hpp

#include "scenes.h"

class OpeningScene : public GScene
{
public:
	OpeningScene();

	void startDialog();

	virtual inline GScene* getReplacementScene() { return Node::ccCreate<OpeningScene>(); }
};

#endif /* Library_hpp */
