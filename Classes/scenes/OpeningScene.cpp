//
//  Library.cpp
//  Koumachika
//
//  Created by Toni on 1/22/19.
//
//

#include "Prefix.h"

#include "functional.hpp"
#include "OpeningScene.hpp"

OpeningScene::OpeningScene() :
	GScene("", {})
{
	multiInit += wrap_method(OpeningScene, startDialog, this);
}

void OpeningScene::startDialog()
{
	createDialog("dialogs/opening_scene", false);
}
