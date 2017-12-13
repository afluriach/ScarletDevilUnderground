//
//  SceneMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

#include "Collect.h"
#include "Garden.hpp"
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

GenericPlayScene(BlockRoom)
GenericPlayScene(StalkerRoom)

GenericPlayScene(Clearing)
GenericPlayScene(ClearingPath)

GenericPlayScene(GardenEmpty)
GenericPlayScene(GardenPath)

GenericPlayScene(Library)

const unordered_map<string,GScene::AdapterType> GScene::adapters = boost::assign::map_list_of
    entry_same(BlockRoom)
    entry_same(Collect)
    entry_same(StalkerRoom)
    entry_same(GardenBlooming)
    entry_same(GardenEmpty)
    entry_same(GardenPath)
    entry_same(Library)
    entry_same(LibraryOpening)
    entry_same(MapScene)
    entry_same(TitleMenu)

    entry_same(Clearing)
    entry_same(ClearingPath)
;