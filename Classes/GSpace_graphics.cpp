//
//  GSpace_graphics.cpp
//  Koumachika
//
//  Created by Toni on 2/21/19.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "scenes.h"

//BEGIN GRAPHICS

LightID GSpace::addLightSource(CircleLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

LightID GSpace::addLightSource(AmbientLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

LightID GSpace::addLightSource(ConeLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

LightID GSpace::addLightSource(SpriteLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

void GSpace::updateLightSource(LightID id, ConeLightArea light)
{
	addLightmapAction([this, id, light]()->void {
		gscene->updateLightSource(id, light);
	});
}

void GSpace::removeLightSource(LightID id)
{
	addLightmapAction([this, id]()->void {
		gscene->removeLightSource(id);
	});
}

void GSpace::setLightSourcePosition(LightID id, SpaceVect pos)
{
	addLightmapAction([this, id, pos]()->void {
		gscene->setLightSourcePosition(id, pos);
	});
}

void GSpace::setLightSourceAngle(LightID id, SpaceFloat a)
{
	addLightmapAction([this, id, a]()->void {
		gscene->setLightSourceAngle(id, a);
	});
}

SpriteID GSpace::createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, path, sceneLayer, pos, zoom]() -> void {
		gscene->createSprite(id, path, sceneLayer, pos, zoom);
	});

	return id;
}

SpriteID GSpace::createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, name, frameCount, duration, sceneLayer, pos, zoom]() -> void {
		gscene->createLoopAnimation(id, name, frameCount, duration, sceneLayer, pos, zoom);
	});

	return id;
}

SpriteID GSpace::createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, sceneLayer, pos, zoom]() -> void {
		gscene->createDrawNode(id, sceneLayer, pos, zoom);
	});

	return id;
}

SpriteID GSpace::createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, path, isAgentAnimation, sceneLayer, pos, zoom]() -> void {
		gscene->createAgentSprite(id, path, isAgentAnimation, sceneLayer, pos, zoom);
	});

	return id;
}

void GSpace::createDamageIndicator(float val, SpaceVect pos)
{
	addSpriteAction([this, val, pos]()->void {
		gscene->createDamageIndicator(val, pos);
	});
}

void GSpace::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	addSpriteAction([this, id, path, isAgentAnimation]() -> void {
		gscene->loadAgentAnimation(id, path, isAgentAnimation);
	});
}

void GSpace::setAgentAnimationDirection(SpriteID id, Direction d)
{
	addSpriteAction([this, id, d]() -> void {
		gscene->setAgentAnimationDirection(id, d);
	});
}

void GSpace::setAgentAnimationFrame(SpriteID id, int frame)
{
	addSpriteAction([this, id, frame]() -> void {
		gscene->setAgentAnimationFrame(id, frame);
	});
}

void GSpace::clearDrawNode(SpriteID id)
{
	addSpriteAction([this, id]() -> void {
		gscene->clearDrawNode(id);
	});
}

void GSpace::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	addSpriteAction([this, id, lowerLeft, upperRight, color]() -> void {
		gscene->drawSolidRect(id, lowerLeft, upperRight, color);
	});
}

void GSpace::drawSolidCone(SpriteID id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color)
{
	addSpriteAction([this, id, center, radius, startAngle, endAngle, segments, color]() -> void {
		gscene->drawSolidCone(id, center, radius, startAngle, endAngle, segments, color);
	});
}

void GSpace::drawSolidCircle(SpriteID id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color)
{
	addSpriteAction([this, id, center, radius, angle, segments, color]() -> void {
		gscene->drawSolidCircle(id, center, radius, angle, segments, color);
	});
}

void GSpace::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	addSpriteAction([this, id, generator]() -> void {
		gscene->runSpriteAction(id, generator);
	});
}

void GSpace::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	addSpriteAction([this, id, action]() -> void {
		gscene->stopSpriteAction(id, action);
	});
}

void GSpace::stopAllSpriteActions(SpriteID id)
{
	addSpriteAction([this, id]() -> void {
		gscene->stopAllSpriteActions(id);
	});
}

void GSpace::removeSprite(SpriteID id)
{
	addSpriteAction([this, id]() -> void {
		gscene->removeSprite(id);
	});
}

void GSpace::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	addSpriteAction([this, id, generator]() -> void {
		gscene->removeSpriteWithAnimation(id, generator);
	});
}

void GSpace::setSpriteVisible(SpriteID id, bool val)
{
	addSpriteAction([this, id, val]() -> void {
		gscene->setSpriteVisible(id, val);
	});
}

void GSpace::setSpriteOpacity(SpriteID id, unsigned char op)
{
	addSpriteAction([this, id, op]() -> void {
		gscene->setSpriteOpacity(id, op);
	});
}

void GSpace::setSpriteTexture(SpriteID id, string path)
{
	addSpriteAction([this, id, path]()->void {
		gscene->setSpriteTexture(id, path);
	});
}

void GSpace::setSpriteAngle(SpriteID id, float cocosAngle)
{
	addSpriteAction([this, id, cocosAngle]() -> void {
		gscene->setSpriteAngle(id, cocosAngle);
	});
}

void GSpace::setSpritePosition(SpriteID id, Vec2 pos)
{
	addSpriteAction([this, id, pos]() -> void {
		gscene->setSpritePosition(id, pos);
	});
}

void GSpace::setSpriteZoom(SpriteID id, float zoom)
{
	addSpriteAction([this, id, zoom]() -> void {
		gscene->setSpriteZoom(id, zoom);
	});
}

void GSpace::setSpriteColor(SpriteID id, Color3B color)
{
	addSpriteAction([this, id, color]()->void {
		gscene->setSpriteColor(id, color);
	});
}

void GSpace::clearSubroomMask(unsigned int roomID)
{
	addSpriteAction([this, roomID]() -> void {
		gscene->clearSubroomMask(roomID);
	});
}

void GSpace::addLightmapAction(function<void()> f)
{
	sceneActions.push_back(make_pair(f, GScene::updateOrder::lightmapUpdate));
}

void GSpace::addSpriteAction(function<void()> f)
{
	sceneActions.push_back(make_pair(f, GScene::updateOrder::spriteUpdate));
}

//END GRAPHICS
