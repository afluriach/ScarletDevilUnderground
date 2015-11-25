//
//  menu.cpp
//  FlansBasement
//
//  Created by Toni on 11/16/15.
//
//

#include "Prefix.h"

#include "menu.h"

//scenes
#include "LibraryOpening.h"
#include "PlayScene.hpp"

const std::vector<std::string> SceneSelect::sceneTitles = list_of_typed(
    ("Play Scene")
    ("Library Opening")
    ("Back"),
    std::vector<std::string>
);

template <typename T>
SceneSelect::SceneLaunchAdapter sceneLaunchAdapter(){
    return []() -> void { app->pushScene<T>(); };
}

const std::vector<SceneSelect::SceneLaunchAdapter> SceneSelect::sceneActions = list_of_typed(
    (sceneLaunchAdapter<PlayScene>())
    (sceneLaunchAdapter<LibraryOpening>())
    (SceneSelect::back),
    std::vector<SceneSelect::SceneLaunchAdapter>
);
