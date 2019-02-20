//
//  ObjectMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/22/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "AreaSensor.hpp"
#include "Bat.hpp"
#include "Block.hpp"
#include "Bullet.hpp"
#include "CollectGlyph.hpp"
#include "Collectibles.hpp"
#include "Desk.hpp"
#include "Door.hpp"
#include "EffectArea.hpp"
#include "EnemyBullet.hpp"
#include "Facer.hpp"
#include "Fairy.hpp"
#include "Follower.hpp"
#include "FloorSegment.hpp"
#include "Flower.h"
#include "Goal.hpp"
#include "Glyph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Launcher.hpp"
#include "Marisa.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"
#include "Pyramid.hpp"
#include "Reimu.hpp"
#include "Sakuya.hpp"
#include "SakuyaNPC.hpp"
#include "Scorpion.hpp"
#include "Sign.hpp"
#include "Slime.hpp"
#include "Spawner.hpp"
#include "Stalker.hpp"
#include "TeleportPad.hpp"
#include "Tewi.hpp"
#include "Torch.hpp"
#include "Upgrade.hpp"
#include "Wall.hpp"

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
constexpr GObject::AdapterType consAdapter()
{
    return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* { return new T(space,id,args); };
}

//Inventory adaptor: will return nullptr if the item has already been acquired,
//meaning item will not be added.
template <typename T>
constexpr GObject::AdapterType itemAdapter(const string& name)
{
    return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
        if(App::crntState->itemRegistry.find(name) != App::crntState->itemRegistry.end())
            return nullptr;
        else return new T(space,id,args);
    };
}

GObject::AdapterType playerAdapter()
{
	return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		switch (App::crntPC) {
		case PlayerCharacter::flandre:
			return new FlandrePC(space,id,args);
		case PlayerCharacter::rumia:
			return new RumiaPC(space,id,args);
		case PlayerCharacter::cirno:
			return new CirnoPC(space,id,args);
		default:
			return nullptr;
		}
	};
}

template<class C>
GObject::AdapterType collectibleAdapter(collectible_id id)
{
	return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		SpaceVect pos = SpaceVect(getFloat(args, "pos_x"), getFloat(args, "pos_y"));
		return new C(space, id, pos);
	};
}

#define entry(name,cls) {name, consAdapter<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define item_entry(name,cls,itemKey) {name, itemAdapter<cls>(#itemKey)}
#define item_entry_same(cls) item_entry(#cls,cls,cls)

#define collectible_entry(name,id) {#name, collectibleAdapter<name>(collectible_id::id)}

const unordered_map<string, GObject::AdapterType> GObject::adapters = {
	entry_same(AgilityUpgrade),
	entry_same(Bat),
	entry_same(BlueFairy),
	entry_same(BreakableWall),
	entry_same(CollectGlyph),
	entry_same(CollectMarisa),
	entry_same(Barrier),
	entry_same(Block),
	entry_same(DarknessArea),
	entry_same(Desk),
	entry_same(DirtFloorCave),
	entry_same(Door),
	entry_same(Facer),
	entry_same(Fairy1),
	entry_same(Fairy2),
	entry_same(Flower),
	entry_same(Follower),
	item_entry_same(ForestBook1),
	entry_same(ForestMarisa),
	entry_same(GenericAgent),
	entry_same(Glyph),
	entry_same(Goal),
	entry_same(GrassFloor),
	item_entry_same(GraveyardBook1),
	item_entry_same(GraveyardKey),
	entry_same(GreenFairy),
	collectible_entry(Health1, health1),
	collectible_entry(Health2, health2),
	entry_same(HiddenSubroomSensor),
	entry_same(HPUpgrade),
	entry_same(IceFairy),
	entry_same(IceFloor),
	entry_same(IcePlatform),
	collectible_entry(Key, key),
	entry_same(Launcher),
	collectible_entry(Magic1, magic1),
	collectible_entry(Magic2, magic2),
	entry_same(MineFloor),
	entry_same(MovingPlatform),
	entry_same(MPUpgrade),
	entry_same(Patchouli),
	entry_same(PatchouliEnemy),
	entry_same(Pitfall),
	collectible_entry(Power1, power1),
	collectible_entry(Power2, power2),
	entry_same(PowerUpgrade),
	entry_same(PressurePlate),
	entry_same(Pyramid),
	entry_same(RedFairy),
	entry_same(Reimu),
	entry_same(Sakuya),
	entry_same(SakuyaNPC),
	entry_same(SandFloor),
	entry_same(Scorpion1),
	entry_same(Scorpion2),
	entry_same(Sign),
	entry_same(Slime1),
	entry_same(Slime2),
	entry_same(Spawner),
	entry_same(Stalker),
	entry_same(StoneFloor),
	entry_same(SunArea),
	entry_same(TeleportPad),
	entry_same(Tewi),
	entry_same(Torch),
	entry_same(Wall),
	entry_same(ZombieFairy),

	{ "Player", playerAdapter() }
};

const set<type_index> GSpace::trackedTypes = {
	typeid(Door),
	typeid(Spawner),
	typeid(TeleportPad),

	//virtual tracked types
	typeid(EnemyBullet),
	typeid(FloorSegment),
	typeid(Wall),
};

const set<type_index> GSpace::enemyTypes = {
	typeid(Fairy1),
	typeid(Fairy2),
	typeid(IceFairy),
	
	typeid(ForestMarisa),
	typeid(PatchouliEnemy),
	typeid(Sakuya),

	typeid(Scorpion1),
	typeid(Scorpion2),
	typeid(Slime1),
	typeid(Slime2),
	typeid(Stalker),
};

#define _nameTypeEntry(cls) {#cls, typeid(cls)}

const map<string,type_index> GSpace::enemyNameTypeMap = {
	_nameTypeEntry(Fairy1),
	_nameTypeEntry(Fairy2),
	_nameTypeEntry(IceFairy),

	_nameTypeEntry(Scorpion1),
	_nameTypeEntry(Scorpion2),
	_nameTypeEntry(Slime1),
	_nameTypeEntry(Slime2),
	_nameTypeEntry(Stalker),
};
