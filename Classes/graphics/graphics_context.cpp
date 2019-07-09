//
//  graphics_context.cpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "GAnimation.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GraphicsNodes.hpp"
#include "graphics_types.h"
#include "GScene.hpp"

void graphics_context::update()
{
	CCRect cameraPix = scene->getCameraArea().toPixelspace();

	for (GAnimation* anim : animationNodes | boost::adaptors::map_values) {
		anim->update();
	}

	for (auto it = autoremoveLightTimers.begin(); it != autoremoveLightTimers.end(); ) {
		timerDecrement(it->second);

		if (it->second <= 0.0) {
			removeLightSource(it->first);
			it = autoremoveLightTimers.erase(it);
		}
		else {
			++it;
		}
	}

	for (auto entry : lightmapNodes)
	{
		bool visible = cameraPix.intersectsRect(makeRect(
			entry.second->getPosition(),
			entry.second->getContentSize()
		));
		entry.second->setVisible(visible);
	}

	for (auto& entry : lightmapNoise)
	{
		SpaceVect fieldPos = SpaceVect::ray(entry.second.radius, entry.second.crntAngle);
		//double output = (lightmapPerlinNoise.GetValue(fieldPos.x, fieldPos.y, 0.0) + 1.0) * 0.5;
		double pout = lightmapPerlinNoise.GetValue(fieldPos.x, fieldPos.y, 0.0);
		double intensity = (pout + 1.0) * 0.5 * (1.0 - entry.second.baseIntensity) + entry.second.baseIntensity;

		setLightSourceColor(entry.first, entry.second.baseColor * intensity);

		entry.second.crntAngle += 1.0 / entry.second.cycleInterval * app::params.secondsPerFrame;
		if (entry.second.crntAngle >= float_pi * 2.0) {
			entry.second.crntAngle -= float_pi * 2.0;
		}
	}

}

SpriteID graphics_context::getSpriteID()
{
	return nextSpriteID.fetch_add(1);
}

LightID graphics_context::getLightID()
{
	return nextLightID.fetch_add(1);
}

Color4F graphics_context::getLightSourceColor(LightID id)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		if (auto cone = dynamic_cast<ConeShader*>(it->second)) {
			return cone->getLightColor();
		}
		else if (auto radial = dynamic_cast<RadialGradient*>(it->second)) {
			return radial->getColor4F();
		}
		else if (auto ambient = dynamic_cast<AmbientLightNode*>(it->second)) {
			return ambient->getLightColor();
		}
	}

	log("getLightSourceColor: light %d does not exist.", id);
	return Color4F::BLACK;
}

Node* graphics_context::getSpriteAsNode(SpriteID id)
{
	auto it = graphicsNodes.find(id);
	if (it != graphicsNodes.end()) {
		return it->second;
	}
	return nullptr;
}

void graphics_context::_removeSprite(SpriteID id)
{
	graphicsNodes.erase(id);
	animationNodes.erase(id);
}

void graphics_context::addPolyLightSource(LightID id, shared_ptr<LightArea> light, SpaceVect pos, SpaceFloat angle)
{
	if (!light) return;

	_polyAddLight<CircleLightArea>(id, light, pos, angle);
	_polyAddLight<AmbientLightArea>(id, light, pos, angle);
	_polyAddLight<ConeLightArea>(id, light, pos, angle);
	_polyAddLight<SpriteLightArea>(id, light, pos, angle);
}

void graphics_context::addLightSource(LightID id, CircleLightArea light, SpaceVect pos, SpaceFloat angle)
{
	bool mask = light.color.a < 0.0f;
	if (mask)
		light.color.a *= -1.0f;

	RadialGradient* g = Node::ccCreate<RadialGradient>(
		light.color,
		Color4F(0.0f, 0.0f, 0.0f, 0.0f),
		light.radius * app::pixelsPerTile,
		Vec2::ZERO,
		light.flood
		);
	CCSize bounds = CCSize(light.radius, light.radius) * 2.0f * app::pixelsPerTile;

	if (mask)
		g->setBlendFunc(BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });
	else
		g->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });

	g->setPosition(toCocos(pos) * app::pixelsPerTile);
	g->setContentSize(bounds);
	scene->getLayer(GScene::sceneLayers::lightmap)->addChild(g, mask ? 1 : 0);

	lightmapNodes.insert_or_assign(id, g);
}

void graphics_context::addLightSource(LightID id, AmbientLightArea light, SpaceVect pos, SpaceFloat angle)
{
	AmbientLightNode* node = Node::ccCreate<AmbientLightNode>(light);
	node->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	node->setPosition(toCocos(pos) * app::pixelsPerTile);
	scene->getLayer(GScene::sceneLayers::lightmap)->addChild(node);
	lightmapNodes.insert_or_assign(id, node);
}

void graphics_context::addLightSource(LightID id, ConeLightArea light, SpaceVect pos, SpaceFloat angle)
{
	ConeShader* cs = Node::ccCreate<ConeShader>(
		light.color,
		light.radius * app::pixelsPerTile,
		Vec2::ZERO,
		light.angleWidth,
		0.0
		);

	cs->setPosition(toCocos(pos) * app::pixelsPerTile);
	cs->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	cs->setContentSize(CCSize(light.radius, light.radius) * 2.0f * app::pixelsPerTile);
	scene->getLayer(GScene::sceneLayers::lightmap)->addChild(cs);

	lightmapNodes.insert_or_assign(id, cs);
}

void graphics_context::addLightSource(LightID id, SpriteLightArea light, SpaceVect pos, SpaceFloat angle)
{
	Sprite* s = Sprite::create(light.texName);

	s->setPosition(toCocos(pos) * app::pixelsPerTile);
	s->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	s->setScale(light.scale);
	s->setColor(toColor3B(light.color));
	scene->getLayer(GScene::sceneLayers::lightmap)->addChild(s);

	lightmapNodes.insert_or_assign(id, s);
}

void graphics_context::removeLightSource(LightID id)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		scene->getLayer(GScene::sceneLayers::lightmap)->removeChild(it->second);
		lightmapNodes.erase(it);
	}
	else {
		log("removeLightSource: unknown light source %u.", to_uint(id));
	}
	lightmapNoise.erase(id);
}

void graphics_context::setLightSourcePosition(LightID id, SpaceVect pos)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		it->second->setPosition(toCocos(pos)*app::pixelsPerTile);
	}
}

void graphics_context::setLightSourceAngle(LightID id, SpaceFloat a)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		it->second->setRotation(toCocosAngle(a));
	}
}

void graphics_context::setLightSourceColor(LightID id, Color4F color)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		if (auto cone = dynamic_cast<ConeShader*>(it->second)) {
			cone->setLightColor(color);
		}
		else if (auto radial = dynamic_cast<RadialGradient*>(it->second)) {
			radial->setColor4F(color);
		}
	}
}

void graphics_context::setLightSourceNoise(LightID id, perlin_light_state noise)
{
	noise.crntAngle = noise.startAngle;
	lightmapNoise.insert_or_assign(id, noise);
}

void graphics_context::autoremoveLightSource(LightID id, float seconds)
{
	autoremoveLightTimers.insert_or_assign(id, seconds);
}

void graphics_context::createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	Sprite* s = Sprite::create(path);
	scene->getSpaceLayer()->positionAndAddNode(s, to_int(sceneLayer), pos, zoom);
	graphicsNodes.insert_or_assign(id, s);
}

void graphics_context::createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	TimedLoopAnimation* anim = Node::ccCreate<TimedLoopAnimation>();
	anim->loadAnimation(name, frameCount, duration);
	scene->getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
	graphicsNodes.insert_or_assign(id, anim);
	animationNodes.insert_or_assign(id, anim);
}

void graphics_context::createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	DrawNode* dn = DrawNode::create();
	graphicsNodes.insert_or_assign(id, dn);
	scene->getSpaceLayer()->positionAndAddNode(dn, to_int(sceneLayer), pos, zoom);
}

void graphics_context::createAgentSprite(
	SpriteID id,
	string _sprite,
	SpaceFloat agentSize,
	GraphicsLayer sceneLayer,
	Vec2 pos
){
	sprite_properties sprite = app::getSprite(_sprite);

	float zoom = getSpriteZoom(sprite, agentSize);
	if (sprite.size != make_pair(3, 4) && sprite.size != make_pair(4, 4)) {
		log("Invalid agent animation size %d,%d.", sprite.size.first, sprite.size.second);
		return;
	}

	PatchConAnimation* anim = Node::ccCreate<PatchConAnimation>();
	anim->loadAnimation(_sprite);
	graphicsNodes.insert_or_assign(id, anim);
	scene->getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
}

void graphics_context::createDamageIndicator(float val, SpaceVect pos)
{
	string s = (val >= 0.0f) ? floatToRoundedString(val, 4.0f) : "-";

	Label* label = createTextLabel(s, 18);
	Vec2 startPos = toCocos(pos + SpaceVect(0.0, 0.5)) * app::pixelsPerTile;
	label->setPosition(startPos);
	label->setScale(2.0f);

	scene->getSpaceLayer()->addChild(label, to_uint(GraphicsLayer::agentOverlay));
	label->runAction(damageIndicatorAction(startPos)());
}

void graphics_context::createAgentBodyShader(
	SpriteID id,
	GraphicsLayer layer,
	Color4F bodyColor,
	Color4F coneColor,
	float bodyRadius,
	float coneRadius,
	float thickness,
	const Vec2 position
) {
	AgentBodyShader* shader = Node::ccCreate<AgentBodyShader>(
		bodyColor,
		coneColor,
		bodyRadius,
		coneRadius,
		thickness,
		Vec2::ZERO
		);
	shader->setContentSize(CCSize(coneRadius, coneRadius) * 2.0f * app::pixelsPerTile);

	graphicsNodes.insert_or_assign(id, shader);
	scene->getSpaceLayer()->positionAndAddNode(shader, to_int(layer), position, 1.0f);
}

void graphics_context::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(generator());
	}
}

void graphics_context::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	nodeAction(id, &Node::stopActionByTag, to_int(action));
}

void graphics_context::stopAllSpriteActions(SpriteID id)
{
	nodeAction(id, &Node::stopAllActions);
}

void graphics_context::removeSprite(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->removeFromParent();
		_removeSprite(id);
	}
}

void graphics_context::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(Sequence::createWithTwoActions(generator(), RemoveSelf::create()));
	}
}

void graphics_context::setSpriteTexture(SpriteID id, string path)
{
	nodeAction<Sprite, const string&>(id, &Sprite::setTexture, path);
}

void graphics_context::setSpritePosition(SpriteID id, Vec2 pos)
{
	nodeAction<Node, const Vec2&>(id, &Node::setPosition, pos);
}

void graphics_context::setSpriteZoom(SpriteID id, float zoom)
{
	nodeAction<Node, float>(id, &Node::setScale, zoom);
}

void graphics_context::spriteSpatialUpdate(vector<sprite_update> spriteUpdates)
{
	for (auto entry : spriteUpdates)
	{
		if (entry.spriteID != 0) {
			Node* sprite = getSpriteAsNode(entry.spriteID);
			if (sprite) {
				sprite->setPosition(entry.pos);
				if (entry.rotateSprite)
					sprite->setRotation(entry.angle);
			}
		}
		if (entry.drawNodeID != 0) {
			Node* drawNode = getSpriteAsNode(entry.drawNodeID);
			if (drawNode) {
				drawNode->setPosition(entry.pos);
				drawNode->setRotation(entry.angle);
			}
		}
		if (entry.lightID != 0) {
			Node* light = getOrDefault<LightID, Node*>(lightmapNodes, entry.lightID, nullptr);
			if (light) {
				light->setPosition(entry.pos);
				light->setRotation(entry.angle);
			}
		}

		if (entry.fadeOut) {
			stopSpriteAction(entry.spriteID, cocos_action_tag::object_fade);
			runSpriteAction(entry.spriteID, objectFadeOut(objectFadeOutTime, objectFadeOpacity));
		}
		if (entry.fadeIn) {
			stopSpriteAction(entry.spriteID, cocos_action_tag::object_fade);
			runSpriteAction(entry.spriteID, objectFadeOut(objectFadeInTime, 255));
		}
	}
}

void graphics_context::clearSubroomMask(unsigned int roomID)
{
	roomMasks.at(roomID)->setVisible(false);
}
