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
#include "Forest.hpp"
#include "Graveyard.hpp"
#include "IceCave.hpp"
#include "Library.hpp"
#include "LibraryOpening.h"
#include "menu_scenes.h"
#include "Mine.hpp"
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
	PlaySceneMapName(B1),
	PlaySceneMapName(BlockRoom),
	PlaySceneMapName(Clearing),
	PlaySceneMapName(ClearingPath),
	entry_same(Collect),
	entry_same(Desert),
	PlaySceneMapName(F2),
	PlaySceneMapName(FacerFloor),
	PlaySceneMapName(Flock),
	entry_same(Forest),
	PlaySceneMapName(FR1),
	PlaySceneMapName(GardenEmpty),
	PlaySceneMapName(GardenPath),
	PlaySceneMapName(G1),
	entry_same(Graveyard1),
	entry_same(Graveyard2),
	entry_same(Graveyard3),
	entry_same(Graveyard4),
	entry_same(IceCave),
	entry_same(Library),
	entry_same(LibraryOpening),
	{"Mansion", []()->GScene* { return App::runOverworldScene(); }},
	PlaySceneMapName(MarisaRoom),
	entry_same(Mine),
	entry_same(OpeningScene),
	PlaySceneMapName(OrbTest),
	PlaySceneMapName(RF),
	PlaySceneMapName(SakuyaRoom),
	PlaySceneMapName(StalkerRoom),
	entry_same(TitleMenuScene),
	PlaySceneMapName(Wander)
};

const unordered_map<ChamberID, PlayScene::AdapterType> PlayScene::adapters = {
	{ChamberID::graveyard1, adapterPS<Graveyard1>() },
	{ChamberID::graveyard2, adapterPS<Graveyard2>() },
	{ChamberID::graveyard3, adapterPS<Graveyard3>() },
	{ChamberID::graveyard4, adapterPS<Graveyard4>() },
	{ChamberID::forest1, adapterPS<Forest>() },
	{ChamberID::desert1, adapterPS<Desert>() },
	{ChamberID::mine1, adapterPS<Mine>() },
};
