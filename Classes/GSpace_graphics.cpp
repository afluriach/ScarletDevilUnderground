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

void GSpace::updateLightSource(LightID id, ConeLightArea light)
{
	addLightmapAction(&GScene::updateLightSource, id, light);
}

void GSpace::removeLightSource(LightID id)
{
	addLightmapAction(&GScene::removeLightSource, id);
}

void GSpace::setLightSourcePosition(LightID id, SpaceVect pos)
{
	addLightmapAction(&GScene::setLightSourcePosition, id, pos);
}

void GSpace::setLightSourceAngle(LightID id, SpaceFloat a)
{
	addLightmapAction(&GScene::setLightSourceAngle, id, a);
}

SpriteID GSpace::createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	return createSprite(&GScene::createSprite, path, sceneLayer, pos, zoom);
}

SpriteID GSpace::createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	return createSprite(&GScene::createLoopAnimation, name, frameCount, duration, sceneLayer, pos, zoom);
}

SpriteID GSpace::createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	return createSprite(&GScene::createDrawNode, sceneLayer, pos, zoom);
}

SpriteID GSpace::createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	return createSprite(&GScene::createAgentSprite, path, isAgentAnimation, sceneLayer, pos, zoom);
}

void GSpace::createDamageIndicator(float val, SpaceVect pos)
{
	addSpriteAction(&GScene::createDamageIndicator, val, pos);
}

SpriteID GSpace::createAgentBodyShader(
	GraphicsLayer layer,
	const Color4F& bodyColor, const Color4F& coneColor,
	float bodyRadius, float coneRadius,
	float thickness, const Vec2& position,
	float startAngle, float endAngle
){
	return createSprite(
		&GScene::createAgentBodyShader,
		layer,
		bodyColor, coneColor,
		bodyRadius, coneRadius,
		thickness, position,
		startAngle, endAngle
	);
}


void GSpace::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	addSpriteAction(&GScene::loadAgentAnimation, id, path, isAgentAnimation);
}

void GSpace::setAgentAnimationDirection(SpriteID id, Direction d)
{
	addSpriteAction(&GScene::setAgentAnimationDirection, id, d);
}

void GSpace::setAgentAnimationFrame(SpriteID id, int frame)
{
	addSpriteAction(&GScene::setAgentAnimationFrame, id, frame);
}

void GSpace::clearDrawNode(SpriteID id)
{
	addSpriteAction(&GScene::clearDrawNode, id);
}

void GSpace::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	addSpriteAction(&GScene::drawSolidRect, id, lowerLeft, upperRight, color);
}

void GSpace::drawSolidCone(SpriteID id, Vec2 center, float radius, float startAngle, float endAngle, unsigned int segments, Color4F color)
{
	addSpriteAction(&GScene::drawSolidCone, id, center, radius, startAngle, endAngle, segments, color);
}

void GSpace::drawSolidCircle(SpriteID id, Vec2 center, float radius, float angle, unsigned int segments, Color4F color)
{
	addSpriteAction(&GScene::drawSolidCircle, id, center, radius, angle, segments, color);
}

void GSpace::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	addSpriteAction(&GScene::runSpriteAction, id, generator);
}

void GSpace::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	addSpriteAction(&GScene::stopSpriteAction, id, action);
}

void GSpace::stopAllSpriteActions(SpriteID id)
{
	addSpriteAction(&GScene::stopAllSpriteActions, id);
}

void GSpace::removeSprite(SpriteID id)
{
	addSpriteAction(&GScene::removeSprite, id);
}

void GSpace::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	addSpriteAction(&GScene::removeSpriteWithAnimation, id, generator);
}

void GSpace::setSpriteVisible(SpriteID id, bool val)
{
	addSpriteAction(&GScene::setSpriteVisible, id, val);
}

void GSpace::setSpriteOpacity(SpriteID id, unsigned char op)
{
	addSpriteAction(&GScene::setSpriteOpacity, id, op);
}

void GSpace::setSpriteTexture(SpriteID id, string path)
{
	addSpriteAction(&GScene::setSpriteTexture, id, path);
}

void GSpace::setSpriteAngle(SpriteID id, float cocosAngle)
{
	addSpriteAction(&GScene::setSpriteAngle, id, cocosAngle);
}

void GSpace::setSpritePosition(SpriteID id, Vec2 pos)
{
	addSpriteAction(&GScene::setSpritePosition, id, pos);
}

void GSpace::setSpriteZoom(SpriteID id, float zoom)
{
	addSpriteAction(&GScene::setSpriteZoom, id, zoom);
}

void GSpace::setSpriteColor(SpriteID id, Color3B color)
{
	addSpriteAction(&GScene::setSpriteColor, id, color);
}

void GSpace::setAgentOverlayAngles(SpriteID id, float startAngle, float endAngle)
{
	addSpriteAction(&GScene::setAgentOverlayAngles, id, startAngle, endAngle);
}

void GSpace::clearSubroomMask(unsigned int roomID)
{
	addSpriteAction(&GScene::clearSubroomMask, roomID);
}

void GSpace::_addLightmapAction(zero_arity_function f)
{
	sceneActions.push_back(make_pair(f, SceneUpdateOrder::lightmapUpdate));
}

void GSpace::_addSpriteAction(zero_arity_function f)
{
	sceneActions.push_back(make_pair(f, SceneUpdateOrder::spriteUpdate));
}

//END GRAPHICS
