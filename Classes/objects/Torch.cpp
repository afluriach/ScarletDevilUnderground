//
//  Torch.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "GScene.hpp"
#include "GSpace.hpp"
#include "Torch.hpp"
#include "value_map.hpp"

const unordered_map<string, Color3B> Torch::colorMap = {
	{ "blue", Color3B(24,48,160) },
	{ "yellow", Color3B(192, 192, 82) },
	{ "white", Color3B(255,255,255) }
};

Torch::Torch(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
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
	baseSpriteID = space->createSprite("sprites/torch.png", GraphicsLayer::ground, getInitialCenterPix(), 0.5f);
	flameSpriteID = space->createSprite("sprites/white_flame.png", GraphicsLayer::overhead, getInitialCenterPix(), 0.5f);
	space->setSpriteColor(flameSpriteID, color);
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

void Torch::interact()
{
	setActive(!isActive);
}
