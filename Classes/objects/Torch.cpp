//
//  Torch.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"
#include "GSpace.hpp"
#include "Torch.hpp"
#include "value_map.hpp"

const unordered_map<string, Color3B> Torch::colorMap = {
	{ "blue", Color3B(24,48,160) },
	{ "yellow", Color3B(192, 192, 82) },
	{ "white", Color3B(255,255,255) }
};

Torch::Torch(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	RegisterUpdate<Torch>(this)
{
	isActive = getBoolOrDefault(args, "active", false);
	intensity = getFloatOrDefault(args, "intensity", intensity);
	lightRadius = getFloatOrDefault(args, "radius", lightRadius);
	flood = getFloatOrDefault(args, "flood", flood);

	colorName = getStringOrDefault(args, "color", "white");
	if (!colorName.empty()) {
		auto it = colorMap.find(colorName);
		if (it != colorMap.end()) {
			color = it->second;
		}
	}
}

void Torch::initializeGraphics()
{
	//	unsigned int createLoopAnimation(string path, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom);

	baseSpriteID = space->createSprite("sprites/torch.png", GraphicsLayer::ground, getInitialCenterPix(), 4.0f);
	flameSpriteID = space->createLoopAnimation(colorName + "_flame", 8, 1.0f, GraphicsLayer::overhead, getInitialCenterPix(), 4.0f);
	space->setSpriteVisible(flameSpriteID, isActive);

	if (isActive) {
		addLightSource();
	}
}

void Torch::setActive(bool active)
{
    isActive = active;

	space->setSpriteVisible(flameSpriteID, active);

	if (active && lightSourceID == 0) {
		addLightSource();
	}
	else if(lightSourceID != 0) {
		space->removeLightSource(lightSourceID);
		lightSourceID = 0;
	}
}

bool Torch::getActive()
{
    return isActive;
}

void Torch::addLightSource()
{
	lightSourceID = space->addLightSource(CircleLightArea{ getPos(),5.0,toColor4F(color)*intensity, flood });
}

void Torch::update()
{
}

void Torch::interact()
{
	setActive(!isActive);
}
