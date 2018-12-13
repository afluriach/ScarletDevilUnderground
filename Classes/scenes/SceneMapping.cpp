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
#include "Mine.hpp"
#include "PlayScene.hpp"

#define entry(name,cls) (name, adapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define PlaySceneMapName(cls) (#cls, playSceneAdapter(#cls)) 

template<typename T>
GScene::AdapterType adapter()
{
    return []() -> void {app->runScene<T>();};
}

GScene::AdapterType playSceneAdapter(const string& name)
{
	return [name]() -> void {app->runScene<PlayScene>(name); };
}

const unordered_map<string,GScene::AdapterType> GScene::adapters = boost::assign::map_list_of
	PlaySceneMapName(B1)
	PlaySceneMapName(BlockRoom)
	PlaySceneMapName(Clearing)
	PlaySceneMapName(ClearingPath)
	entry_same(Collect)
	PlaySceneMapName(FacerFloor)
	PlaySceneMapName(FR1)
    entry_same(GardenBlooming)
	PlaySceneMapName(GardenEmpty)
	PlaySceneMapName(GardenPath)
	PlaySceneMapName(Library)
    entry_same(LibraryOpening)
	entry_same(Mine)
	PlaySceneMapName(SakuyaRoom)
	PlaySceneMapName(StalkerRoom)
	entry_same(TitleMenuScene)
	PlaySceneMapName(Wander)
;
