//
//  SceneMapping.cpp
//  FlansBasement
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

#include "BlockScene.h"
#include "Library.h"
#include "LibraryOpening.h"
#include "PlayScene.hpp"

#define entry(name,cls) (name, adapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

template<typename T>
GScene::AdapterType adapter()
{
    return []() -> void {app->runScene<T>();};
}

const map<string,GScene::AdapterType> GScene::adapters = boost::assign::map_list_of
    entry_same(BlockScene)
    entry_same(Library)
    entry_same(LibraryOpening)
    entry_same(MapScene)
    entry_same(TitleMenu)
;