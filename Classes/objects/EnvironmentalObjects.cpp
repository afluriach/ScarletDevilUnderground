//
//  EnvironmentalObjects.cpp
//  Koumachika
//
//  Created by Toni on 2/23/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "EnvironmentalObjects.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "macros.h"
#include "PlayScene.hpp"
#include "value_map.hpp"

const vector<string> Headstone::damageSprites = {
	"sprites/headstone-damage3.png",
	"sprites/headstone-damage2.png",
	"sprites/headstone-damage1.png",
	"sprites/headstone.png",
};

const vector<float_pair> Headstone::damageIntervals = {
	make_pair(0.0f,0.25f), 
	make_pair(0.25f,0.5f),
	make_pair(0.5f,0.75f),
	make_pair(0.75f, 1.0f),
};

boost::icl::interval_map<float, int> Headstone::intervals = makeIntervalMap(damageIntervals);

bool Headstone::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int level = getIntOrDefault(args, "level", -1);

	if (level == -1)
		return true;
	else
		return !space->getState()->isChamberCompleted(enum_add(ChamberID, graveyard1, level - 1));
}

Headstone::Headstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space,id,args),
RectangleBody(args)
{
	setInitialDirectionOrDefault(args, Direction::up);
	maxHP = getFloatOrDefault(args, "hp", -1.0f);
	hp = maxHP;
}

void Headstone::hit(float damage)
{
	if (maxHP != -1.0f)
	{
		hp -= damage;
		float hpRatio = hp / maxHP;

		//either store previous sprite idx/id or calculate current sprite from current hp ratio

		if (hp <= 0.0f){
			space->removeObject(this);
		}
		else {
			auto it = intervals.find(hpRatio);
			if (it != intervals.end()) {
				if (it->second != spriteIdx) {
					space->setSpriteTexture(spriteID, damageSprites[it->second - 1]);
					spriteIdx = it->second;
				}
			}
			else {
				log("unhandled hp ratio %f", hpRatio);
			}
		}
	}
}

string Headstone::imageSpritePath() const {
	return maxHP != -1.0f ? "sprites/headstone.png" : "sprites/strong_headstone.png";
}

GhostHeadstone::GhostHeadstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Headstone),
	RegisterInit<GhostHeadstone>(this)
{
}

void GhostHeadstone::init()
{
	space->runSpriteAction(spriteID, indefiniteFlickerAction(0.333f, 48, 96));
}

Sapling::Sapling(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
	setInitialAngle(float_pi / 2.0);
}

bool Mushroom::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int objectID = getIntOrDefault(args, "id", -1);
	return objectID != -1 && !space->getState()->isMushroomAcquired(objectID);
}

Mushroom::Mushroom(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
	setInitialAngle(float_pi / 2.0);
	objectID = getIntOrDefault(args, "id", -1);
}

void Mushroom::interact(Player* p)
{
	space->getState()->registerMushroomAcquired(objectID);
	++space->getState()->mushroomCount;
	space->removeObject(this);
	space->addHudAction<string, int>(&HUD::setObjectiveCounter, "sprites/mushroom.png", App::getCrntState()->mushroomCount);
}
