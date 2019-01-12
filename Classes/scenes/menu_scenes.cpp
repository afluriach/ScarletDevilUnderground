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
	GScene("TitleMenuScene", {})
{}

bool TitleMenuScene::init()
{
	GScene::init();

	addChild(Node::ccCreate<TitleMenu>());

	return true;
}

LoadProfileScene::LoadProfileScene() :
	GScene("LoadProfileScene", {})
{}

bool LoadProfileScene::init()
{
	GScene::init();

	addChild(Node::ccCreate<LoadProfileMenu>());

	return true;
}

SceneSelectScene::SceneSelectScene() :
	GScene("SceneSelectScene", {})
{}

bool SceneSelectScene::init()
{
	GScene::init();

	addChild(Node::ccCreate<SceneSelect>());

	return true;
}
