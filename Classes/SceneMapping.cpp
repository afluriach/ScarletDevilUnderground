//
//  SceneMapping.cpp
//  FlansBasement
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

#include "scenes.h"

#include "LibraryOpening.h"
#include "menu.h"
#include "PlayScene.hpp"

#define entry(name,cls) (name, adapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

template<typename T>
GScene::AdapterType adapter()
{
    return []() -> void {app->runScene<T>();};
}

const std::map<std::string,GScene::AdapterType> GScene::adapters = boost::assign::map_list_of
    entry_same(LibraryOpening)
    entry_same(PlayScene)
    entry_same(TitleMenu)
;