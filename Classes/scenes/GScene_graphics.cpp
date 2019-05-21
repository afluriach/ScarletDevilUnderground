//
//  GScene_map.cpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "GAnimation.hpp"
#include "Graphics.h"
#include "GScene.hpp"
#include "enum.h"
#include "macros.h"
#include "menu.h"
#include "types.h"
#include "util.h"

#define sprite_action0(cls, method) spriteAction<cls>(id, &cls::method);
#define sprite_action1(cls, method, a) spriteAction<cls>(id, &cls::method, a);
#define sprite_action3(cls, method, a, b, c) spriteAction<cls>(id, &cls::method, a, b, c);

SpriteID GScene::getSpriteID()
{
	return nextSpriteID.fetch_add(1);
}

LightID GScene::getLightID()
{
	return nextLightID.fetch_add(1);
}

void GScene::addLightSource(LightID id, CircleLightArea light)
{
	bool mask = light.color.a < 0.0f;
	if (mask)
		light.color.a *= -1.0f;

	RadialGradient* g = Node::ccCreate<RadialGradient>(
		light.color,
		Color4F(0.0f,0.0f,0.0f,0.0f),
		light.radius * App::pixelsPerTile,
		Vec2::ZERO,
		light.flood
	);
	CCSize bounds = CCSize(light.radius,light.radius) * 2.0f * App::pixelsPerTile;

	if (mask)
		g->setBlendFunc(BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });
	else
		g->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });

	g->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	g->setContentSize(bounds);
	getLayer(sceneLayers::lightmap)->addChild(g, mask ? 1 : 0);

	lightmapNodes.insert_or_assign(id, g);
}

void GScene::addLightSource(LightID id, AmbientLightArea light)
{
	AmbientLightNode* node = Node::ccCreate<AmbientLightNode>(light);
	node->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	node->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	getLayer(sceneLayers::lightmap)->addChild(node);
	lightmapNodes.insert_or_assign(id, node);
}

void GScene::addLightSource(LightID id, ConeLightArea light)
{
	ConeShader* cs = Node::ccCreate<ConeShader>(
		light.color,
		light.radius * App::pixelsPerTile,
		Vec2::ZERO,
		light.angleWidth,
		0.0
	);

	cs->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	cs->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	cs->setContentSize(CCSize(light.radius,light.radius) * 2.0f * App::pixelsPerTile);
	getLayer(sceneLayers::lightmap)->addChild(cs);

	lightmapNodes.insert_or_assign(id, cs);
}

void GScene::addLightSource(LightID id, SpriteLightArea light)
{
	Sprite* s = Sprite::create(light.texName);

	s->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	s->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	s->setScale(light.scale);
	s->setColor(toColor3B(light.color));
	getLayer(sceneLayers::lightmap)->addChild(s);

	lightmapNodes.insert_or_assign(id, s);
}

void GScene::removeLightSource(LightID id)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		getLayer(sceneLayers::lightmap)->removeChild(it->second);
		lightmapNodes.erase(it);
	}
	else {
		log("removeLightSource: unknown light source %u.", to_uint(id));
	}
	lightmapNoise.erase(id);
}

void GScene::setLightSourcePosition(LightID id, SpaceVect pos)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		it->second->setPosition(toCocos(pos)*App::pixelsPerTile);
	}
}

void GScene::setLightSourceAngle(LightID id, SpaceFloat a)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		it->second->setRotation(toCocosAngle(a));
	}
}

void GScene::setLightSourceColor(LightID id, Color4F color)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		if (auto cone = dynamic_cast<ConeShader*>(it->second)) {
			cone->setLightColor(color);
		}
		else if (auto radial = dynamic_cast<RadialGradient*>(it->second)){
			radial->setColor4F(color);
		}
	}
}

void GScene::setLightSourceNoise(LightID id, perlin_light_state noise)
{
	noise.crntAngle = noise.startAngle;
	lightmapNoise.insert_or_assign(id, noise);
}

void GScene::autoremoveLightSource(LightID id, float seconds)
{
	autoremoveLightTimers.insert_or_assign(id, seconds);
}

void GScene::createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	Sprite* s = Sprite::create(path);
	getSpaceLayer()->positionAndAddNode(s, to_int(sceneLayer), pos, zoom);
	graphicsNodes.insert_or_assign(id, s);
}

void GScene::createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	TimedLoopAnimation* anim = Node::ccCreate<TimedLoopAnimation>();
	anim->loadAnimation(name, frameCount, duration);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
	graphicsNodes.insert_or_assign(id, anim);
	animationNodes.insert_or_assign(id, anim);
}

void GScene::createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	DrawNode* dn = DrawNode::create();
	graphicsNodes.insert_or_assign(id, dn);
	getSpaceLayer()->positionAndAddNode(dn, to_int(sceneLayer), pos, zoom);
}

void GScene::createAgentSprite(SpriteID id, string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	PatchConAnimation* anim = Node::ccCreate<PatchConAnimation>();
	anim->loadAnimation(path, isAgentAnimation);
	graphicsNodes.insert_or_assign(id, anim);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
}

void GScene::createDamageIndicator(float val, SpaceVect pos)
{
	Label* label = createTextLabel(floatToRoundedString(val, 4.0f), 18);
	Vec2 startPos = toCocos(pos + SpaceVect(0.0, 0.5)) * App::pixelsPerTile;
	label->setPosition(startPos);
	label->setScale(2.0f);

	getSpaceLayer()->addChild(label, to_uint(GraphicsLayer::agentOverlay));
	label->runAction(damageIndicatorAction(startPos)());
}

void GScene::createAgentBodyShader(
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
	shader->setContentSize(CCSize(coneRadius,coneRadius) * 2.0f * App::pixelsPerTile);

	graphicsNodes.insert_or_assign(id, shader);
	getSpaceLayer()->positionAndAddNode(shader, to_int(layer), position, 1.0f);
}

void GScene::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	spriteAction<PatchConAnimation, const string&, bool>(id, &PatchConAnimation::loadAnimation, path, isAgentAnimation);
}

void GScene::setAgentAnimationDirection(SpriteID id, Direction d)
{
	sprite_action1(PatchConAnimation, setDirection, d);
}

void GScene::setAgentAnimationFrame(SpriteID id, int frame)
{
	sprite_action1(PatchConAnimation, setFrame, frame);
}

void GScene::setAgentOverlayShieldLevel(SpriteID id, float level)
{
	sprite_action1(AgentBodyShader, setShieldLevel, level);
}

void GScene::clearDrawNode(SpriteID id)
{
	sprite_action0(DrawNode, clear);
}

void GScene::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	spriteAction<DrawNode, const Vec2&, const Vec2&, const Color4F&>
		(id, &DrawNode::drawSolidRect, lowerLeft, upperRight, color);
}

void GScene::drawSolidCone(SpriteID id, Vec2 center, float radius, float startAngle, float endAngle, unsigned int segments, Color4F color)
{
	spriteAction<DrawNode, const Vec2&, float, float, float, unsigned int, const Color4F&>
		(id, &DrawNode::drawSolidCone, center, radius, startAngle, endAngle, segments, color);
}

void GScene::drawSolidCircle(SpriteID id, Vec2 center, float radius, float angle, unsigned int segments, Color4F color)
{
	spriteAction<DrawNode, const Vec2&, float, float, unsigned int, const Color4F&>
		(id, &DrawNode::drawSolidCircle, center, radius, angle, segments, color);
}

void GScene::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(generator());
	}
}

void GScene::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	sprite_action1(Node, stopActionByTag, to_int(action));
}

void GScene::stopAllSpriteActions(SpriteID id)
{
	sprite_action0(Node, stopAllActions);
}

void GScene::removeSprite(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->removeFromParent();
		_removeSprite(id);
	}
}

void GScene::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(Sequence::createWithTwoActions(generator(), RemoveSelf::create()));
	}
}

void GScene::setSpriteVisible(SpriteID id, bool val)
{
	sprite_action1(Node, setVisible, val);
}

void GScene::setSpriteOpacity(SpriteID id, unsigned char op)
{
	sprite_action1(Node, setOpacity, op);
}

void GScene::setSpriteTexture(SpriteID id, string path)
{
	spriteAction<Sprite, const string&>(id, &Sprite::setTexture, path);
}

void GScene::setSpriteAngle(SpriteID id, float cocosAngle)
{
	sprite_action1(Node, setRotation, cocosAngle);
}

void GScene::setSpritePosition(SpriteID id, Vec2 pos)
{
	spriteAction<Sprite, const Vec2&>(id, &Sprite::setPosition, pos);
}

void GScene::setSpriteZoom(SpriteID id, float zoom)
{
	sprite_action1(Node, setScale, zoom);
}

void GScene::setSpriteColor(SpriteID id, Color3B color)
{
	spriteAction<Node, const Color3B&>(id, &Node::setColorRecursive, color);
}

void GScene::spriteSpatialUpdate(vector<sprite_update> spriteUpdates)
{
	for (auto entry : spriteUpdates)
	{
		if (entry.spriteID != 0) {
			Node* sprite = getSpriteAsNode(entry.spriteID);
			if (sprite) {
				sprite->setPosition(entry.pos);
				if(entry.rotateSprite)
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

void GScene::clearSubroomMask(unsigned int roomID)
{
	roomMasks.at(roomID)->setVisible(false);
}

Node* GScene::getSpriteAsNode(SpriteID id)
{
	auto it = graphicsNodes.find(id);
	if (it != graphicsNodes.end()) {
		return it->second;
	}
	return nullptr;
}

void GScene::_removeSprite(SpriteID id)
{
	graphicsNodes.erase(id);
	animationNodes.erase(id);
}

void GScene::renderSpace()
{
	spaceRender->setVisible(display != displayMode::lightmap);
	lightmapRender->setVisible(display != displayMode::base);
	lightmapBackground->setVisible(display == displayMode::lightmap);

	for (GAnimation* anim : animationNodes | boost::adaptors::map_values) {
		anim->update();
	}

	redrawLightmap();
}

void GScene::redrawLightmap()
{
	lightmapDrawNode->clear();

	CCRect cameraPix = getCameraArea().toPixelspace();

	if (display == displayMode::lightmap) {
		lightmapDrawNode->drawSolidRect(
			cameraPix.getLowerCorner(),
			cameraPix.getUpperCorner(),
			Color4F::BLACK
		);
	}

	for (auto it = autoremoveLightTimers.begin(); it != autoremoveLightTimers.end(); ) {
		it->second -= App::secondsPerFrame;

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

		entry.second.crntAngle += 1.0 / entry.second.cycleInterval * App::secondsPerFrame;
		if (entry.second.crntAngle >= float_pi * 2.0) {
			entry.second.crntAngle -= float_pi * 2.0;
		}
	}
}

void GScene::cycleDisplayMode()
{
	enum_increment(displayMode, display);
	if (display == displayMode::end) {
		display = displayMode::begin;
	}
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer, BlendFunc blend)
{
	RenderTexture* rt = initRenderTexture(sceneLayer);
	rt->getSprite()->setBlendFunc(blend);
	return rt;
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer)
{
	RenderTexture* rt = RenderTexture::create(App::width, App::height);
	rt->setPosition(App::width / 2.0f, App::height / 2.0f);
	addChild(rt, to_int(sceneLayer));
	rt->addChild(getLayer(sceneLayer));
	rt->setClearColor(Color4F(0.0f, 0.0f, 0.0f, 0.0f));
	rt->setAutoDraw(true);
	rt->setClearFlags(GL_COLOR_BUFFER_BIT);
	return rt;
}

void GScene::setColorFilter(const Color4F& color)
{
	colorFilterDraw->clear();
	colorFilterDraw->drawSolidRect(-1.0f*Vec2(App::width/2, App::height/2), Vec2(App::width/2, App::height/2), color);
}
