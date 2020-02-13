//
//  SceneMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

#include "LibraryOpening.h"
#include "menu_scenes.h"
#include "OpeningScene.hpp"
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

template<typename T>
constexpr PlayScene::AdapterType adapterPS()
{
	return []() -> PlayScene* { return App::createAndRunSceneAs<T,PlayScene>(); };
}

GScene::AdapterType playSceneAdapter(const string& name)
{
	return [name]() -> GScene* {return App::createAndRunScene<PlayScene>(name); };
}

const unordered_map<string, GScene::AdapterType> GScene::adapters = {
	entry_same(LibraryOpening),
	{"Mansion", []()->GScene* { return App::runOverworldScene(); }},
	entry_same(OpeningScene),
	entry_same(TitleMenuScene),
};
