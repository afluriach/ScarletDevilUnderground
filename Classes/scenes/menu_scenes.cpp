//
//  menu_scenes.cpp
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#include "Prefix.h"

#include "menu_layers.h"
#include "menu_scenes.h"

TitleMenuScene::TitleMenuScene() :
	GScene("")
{}

bool TitleMenuScene::init()
{
	GScene::init();

	addChild(Node::ccCreate<TitleMenu>());

	return true;
}

SceneSelectScene::SceneSelectScene() :
	GScene("")
{}

bool SceneSelectScene::init()
{
	GScene::init();

	addChild(Node::ccCreate<SceneSelect>());

	return true;
}
