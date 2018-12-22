//
//  SceneMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

#include "Collect.h"
#include "Desert.hpp"
#include "Garden.hpp"
#include "IceCave.hpp"
#include "LibraryOpening.h"
#include "menu_scenes.h"
#include "Mine.hpp"
#include "PlayScene.hpp"

#define entry(name,cls) {name, adapter<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define PlaySceneMapName(cls) {#cls, playSceneAdapter(#cls)}

template<typename T>
constexpr GScene::AdapterType adapter()
{
    return []() -> GScene* { return App::createAndRunScene<T>();};
}

GScene::AdapterType playSceneAdapter(const string& name)
{
	return [name]() -> GScene* {return App::createAndRunScene<PlayScene>(name); };
}

const unordered_map<string, GScene::AdapterType> GScene::adapters = {
	PlaySceneMapName(B1),
	PlaySceneMapName(BlockRoom),
	PlaySceneMapName(Clearing),
	PlaySceneMapName(ClearingPath),
	entry_same(Collect),
	entry_same(Desert),
	PlaySceneMapName(FacerFloor),
	PlaySceneMapName(FR1),
	entry_same(GardenBlooming),
	PlaySceneMapName(GardenEmpty),
	PlaySceneMapName(GardenPath),
	entry_same(IceCave),
	PlaySceneMapName(Library),
	entry_same(LibraryOpening),
	PlaySceneMapName(MarisaRoom),
	entry_same(Mine),
	PlaySceneMapName(SakuyaRoom),
	PlaySceneMapName(StalkerRoom),
	entry_same(TitleMenuScene),
	PlaySceneMapName(Wander)
};
