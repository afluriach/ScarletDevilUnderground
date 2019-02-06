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

	pushMenu(Node::ccCreate<TitleMenu>());

	return true;
}
