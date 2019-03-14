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
	RadialGradient* g = Node::ccCreate<RadialGradient>(
		light.color,
		Color4F::BLACK,
		light.radius * App::pixelsPerTile,
		Vec2::ZERO,
		light.flood
	);
	CCSize bounds = CCSize(light.radius,light.radius) * 2.0f * App::pixelsPerTile;

	g->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	g->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	g->setContentSize(bounds);
	getLayer(sceneLayers::lightmap)->addChild(g);

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
		log("removeLightSource: unknown light source %u.", id);
	}
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
	}
}

void GScene::createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	Sprite* s = Sprite::create(path);
	getSpaceLayer()->positionAndAddNode(s, to_int(sceneLayer), pos, zoom);
	crntSprites.insert_or_assign(id, s);
	graphicsNodes.insert_or_assign(id, s);
}

void GScene::createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	TimedLoopAnimation* anim = Node::ccCreate<TimedLoopAnimation>();
	anim->loadAnimation(name, frameCount, duration);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
	animationSprites.insert_or_assign(id, anim);
	graphicsNodes.insert_or_assign(id, anim);
}

void GScene::createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	DrawNode* dn = DrawNode::create();
	drawNodes.insert_or_assign(id, dn);
	graphicsNodes.insert_or_assign(id, dn);
	getSpaceLayer()->positionAndAddNode(dn, to_int(sceneLayer), pos, zoom);
}

void GScene::createAgentSprite(SpriteID id, string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	PatchConAnimation* anim = Node::ccCreate<PatchConAnimation>();
	anim->loadAnimation(path, isAgentAnimation);
	agentSprites.insert_or_assign(id, anim);
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
	const Vec2 position,
	float startAngle,
	float endAngle
) {
	AgentBodyShader* shader = Node::ccCreate<AgentBodyShader>(
		bodyColor,
		coneColor,
		bodyRadius,
		coneRadius,
		thickness,
		Vec2::ZERO,
		startAngle,
		endAngle
	);
	shader->setContentSize(CCSize(coneRadius,coneRadius) * 2.0f * App::pixelsPerTile);

	agentShaders.insert_or_assign(id, shader);
	graphicsNodes.insert_or_assign(id, shader);
	getSpaceLayer()->positionAndAddNode(shader, to_int(layer), position, 1.0f);
}

void GScene::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->loadAnimation(path, isAgentAnimation);
	}
}

void GScene::setAgentAnimationDirection(SpriteID id, Direction d)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->setDirection(d);
	}
}

void GScene::setAgentAnimationFrame(SpriteID id, int frame)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->setFrame(frame);
	}
}

void GScene::clearDrawNode(SpriteID id)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->clear();
	}
}

void GScene::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidRect(lowerLeft, upperRight, color);
	}
}

void GScene::drawSolidCone(SpriteID id, Vec2 center, float radius, float startAngle, float endAngle, unsigned int segments, Color4F color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidCone(center, radius, startAngle, endAngle, segments, color);
	}
}

void GScene::drawSolidCircle(SpriteID id, Vec2 center, float radius, float angle, unsigned int segments, Color4F color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidCircle(center, radius, angle, segments, color);
	}
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
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->stopActionByTag(to_int(action));
	}
}

void GScene::stopAllSpriteActions(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->stopAllActions();
	}
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
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setVisible(val);
	}
}

void GScene::setSpriteOpacity(SpriteID id, unsigned char op)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setOpacity(op);
	}
}

void GScene::setSpriteTexture(SpriteID id, string path)
{
	auto it = crntSprites.find(id);
	if (it != crntSprites.end()) {
		Sprite* s = dynamic_cast<Sprite*>(it->second);
		if (s) {
			s->setTexture(path);
		}
	}
}

void GScene::setSpriteAngle(SpriteID id, float cocosAngle)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setRotation(cocosAngle);
	}
}

void GScene::setSpritePosition(SpriteID id, Vec2 pos)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setPosition(pos);
	}
}

void GScene::setSpriteZoom(SpriteID id, float zoom)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setScale(zoom);
	}
}

void GScene::setSpriteColor(SpriteID id, Color3B color)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setColorRecursive(color);
	}
}

void GScene::setAgentOverlayAngles(SpriteID id, float startAngle, float endAngle)
{
	auto it = agentShaders.find(id);
	if (it != agentShaders.end()) {
		it->second->setAngles(startAngle, endAngle);
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
	crntSprites.erase(id);
	drawNodes.erase(id);
	animationSprites.erase(id);
	agentSprites.erase(id);
	agentShaders.erase(id);
}

void GScene::renderSpace()
{
	spaceRender->setVisible(display != displayMode::lightmap);
	lightmapRender->setVisible(display != displayMode::base);
	lightmapBackground->setVisible(display == displayMode::lightmap);

	for (TimedLoopAnimation* anim : animationSprites | boost::adaptors::map_values) {
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

	for (auto entry : lightmapNodes)
	{
		bool visible = cameraPix.intersectsRect(makeRect(
			entry.second->getPosition(),
			entry.second->getContentSize()
		));
		entry.second->setVisible(visible);
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
