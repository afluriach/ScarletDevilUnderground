//
//  menu_scenes.h
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#ifndef menu_scenes_h
#define menu_scenes_h

#include "scenes.h"

class TitleMenuScene : public GScene
{
public:
	TitleMenuScene();
	virtual bool init();
};

class LoadProfileScene : public GScene
{
public:
	LoadProfileScene();
	virtual bool init();
};


class SceneSelectScene : public GScene
{
public:
	SceneSelectScene();
	virtual bool init();
};

#endif /* menu_scenes_h */
