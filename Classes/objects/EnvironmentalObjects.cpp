//
//  EnvironmentalObjects.cpp
//  Koumachika
//
//  Created by Toni on 2/23/19.
//
//

#include "Prefix.h"

#include "EnvironmentalObjects.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "HUD.hpp"
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
	int level = getIntOrDefault(args, "level", 0);

	if (level == 0)
		return true;
	else
		return !App::crntState->isChamberCompleted("Graveyard" + boost::lexical_cast<string>(level));
}

Headstone::Headstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(make_local_shared<object_params>(space, id, args, true), physics_params(GType::environment, eyeLevelHeightLayers, args, -1.0))
{
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
					space->addGraphicsAction(
						&graphics_context::setSpriteTexture,
						spriteID,
						damageSprites[it->second - 1]
					);
					spriteIdx = it->second;
				}
			}
			else {
				log("unhandled hp ratio %f", hpRatio);
			}
		}
	}
}

string Headstone::getSprite() const {
	return maxHP != -1.0f ? "headstone" : "strongHeadstone";
}

GhostHeadstone::GhostHeadstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Headstone(space,id,args) 
{
}

void GhostHeadstone::init()
{
	GObject::init();

	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		spriteID,
		indefiniteFlickerAction(0.333f, 48, 96).generator
	);
}

Sapling::Sapling(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), physics_params(GType::environment, eyeLevelHeightLayers, args, -1.0))
{
}
