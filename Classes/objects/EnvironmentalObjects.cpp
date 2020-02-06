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
#include "RectangleSensor.hpp"
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
	cost = getIntOrDefault(args, "cost", -1);
}

void GhostHeadstone::init()
{
	GObject::init();

	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		spriteID,
		indefiniteFlickerAction(0.333f, 48, 96).generator
	);

	sensor = new RectangleSensor(
		this,
		space->getArea(getName()),
		enum_bitwise_or(GType, npc, player),
		bind(&GhostHeadstone::onContact, this, placeholders::_1),
		bind(&GhostHeadstone::onEndContact, this, placeholders::_1)
	);
}

void GhostHeadstone::removePhysicsObjects()
{
	GObject::removePhysicsObjects();

	if (sensor) {
		delete sensor;
	}
}

void GhostHeadstone::checkActivate()
{
	if (fairies.size() >= cost)
	{
		//These objects will actually be removed when removals are processed,
		//but it will not run more than once for this sensor.
		for (int i = 0; i < cost; ++i) {
			auto it = fairies.begin();
			if (it != fairies.end()) {
				space->removeObject(*it);
				fairies.erase(it);
			}
		}

		space->removeObject(this);
	}
}

void GhostHeadstone::onContact(GObject* obj)
{
	if (obj->getType() == GType::player) {
		checkActivate();
	}
	else if (obj->getType() == GType::npc && obj->getClsName() == "GhostFairyNPC") {
		fairies.insert(obj);
		checkActivate();
	}
}

void GhostHeadstone::onEndContact(GObject* obj)
{
	if (obj->getType() == GType::npc && obj->getClsName() == "GhostFairyNPC") {
		fairies.erase(obj);
	}
}

Sapling::Sapling(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), physics_params(GType::environment, eyeLevelHeightLayers, args, -1.0))
{
}
