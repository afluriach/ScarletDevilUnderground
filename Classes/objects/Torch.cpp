//
//  Torch.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "Graphics.h"
#include "graphics_context.hpp"
#include "Torch.hpp"
#include "value_map.hpp"

const unordered_map<string, Color3B> Torch::colorMap = {
	{ "blue", Color3B(24,48,160) },
	{ "yellow", Color3B(192, 192, 82) },
	{ "white", Color3B(255,255,255) }
};

const float Torch::darknessDrain = 0.1f;

Torch::Torch(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(
		space,
		id,
		MapParams(),
		MapRectPhys(
			enum_bitwise_or(GType, environment, interactible),
			PhysicsLayers::onGround,
			-1.0
		),
		nullptr
	)
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
	spriteID = space->createSprite(
		&graphics_context::createSprite,
		string("sprites/torch.png"),
		GraphicsLayer::ground,
		getInitialCenterPix(),
		0.5f
	);
	flameSpriteID = space->createSprite(
		&graphics_context::createSprite, 
		string("sprites/white_flame.png"),
		GraphicsLayer::overhead,
		getInitialCenterPix(),
		0.5f
	);
	space->graphicsNodeAction(&Node::setColor, flameSpriteID, color);
	space->graphicsNodeAction(&Node::setVisible, flameSpriteID, isActive);

	if (isActive) {
		addLightSource();
	}
}

sprite_update Torch::updateSprite()
{
	bool crntVisible = space->isInPlayerRoom(getCrntRoomID());

	if (isInFade && crntVisible) {
		space->graphicsNodeAction(&Node::setVisible, flameSpriteID, true);
	}
	else if (!isInFade && !crntVisible) {
		space->graphicsNodeAction(&Node::setVisible, flameSpriteID, false);
	}

	return GObject::updateSprite();
}

void Torch::setActive(bool active)
{
    isActive = active;

	space->graphicsNodeAction(&Node::setVisible, flameSpriteID, active);

	if (active && lightID == 0) {
		addLightSource();
	}
	else if(lightID != 0) {
		space->addGraphicsAction(&graphics_context::removeLightSource, lightID);
		lightID = 0;
	}
}

bool Torch::getActive()
{
    return isActive;
}

void Torch::addLightSource()
{
	lightID = space->addLightSource(
		CircleLightArea::create( lightRadius,toColor4F(color)*intensity, flood ),
		getPos(),
		0.0
	);
	space->addGraphicsAction(
		&graphics_context::setLightSourceNoise,
		lightID,
		perlin_light_state{ toColor4F(color)*intensity, boost::math::float_constants::pi, 0.0f, 4.0f, 0.3f}
	);
}

void Torch::interact(Player* p)
{
	setActive(!isActive);
}
