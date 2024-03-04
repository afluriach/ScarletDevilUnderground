//
//  graphics_context.cpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GraphicsNodes.hpp"
#include "GScene.hpp"

void graphics_context::update()
{
	CCRect cameraPix = scene->getCameraArea().toPixelspace();

	for (Node* anim : animationNodes | boost::adaptors::map_values) {
		anim->update(0.0f);
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
        double pout = lightmapPerlinNoise.noise2D(fieldPos.x, fieldPos.y);
		double intensity = (pout + 1.0) * 0.5 * (1.0 - entry.second.baseIntensity) + entry.second.baseIntensity;
		Node* lightNode = getLight(entry.first);

		lightNode->setColor(toColor3B(entry.second.baseColor * intensity));

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

Node* graphics_context::getSpriteAsNode(SpriteID id)
{
	auto it = graphicsNodes.find(id);
	if (it != graphicsNodes.end()) {
		return it->second;
	}
	return nullptr;
}

Node* graphics_context::getLight(SpriteID id)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		return it->second;
	}
	return nullptr;
}

void graphics_context::_removeSprite(SpriteID id)
{
	graphicsNodes.erase(id);
	animationNodes.erase(id);
}

void graphics_context::addPolyLightSource(SpriteID id, shared_ptr<LightArea> light, SpaceVect pos, SpaceFloat angle)
{
	if (!light) return;

	_polyAddLight<CircleLightArea>(id, light, pos, angle);
	_polyAddLight<AmbientLightArea>(id, light, pos, angle);
	_polyAddLight<ConeLightArea>(id, light, pos, angle);
	_polyAddLight<SpriteLightArea>(id, light, pos, angle);
}

void graphics_context::addLightSource(SpriteID id, CircleLightArea light, SpaceVect pos, SpaceFloat angle)
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

void graphics_context::addLightSource(SpriteID id, AmbientLightArea light, SpaceVect pos, SpaceFloat angle)
{
	AmbientLightNode* node = Node::ccCreate<AmbientLightNode>(light);
	node->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	node->setPosition(toCocos(pos) * app::pixelsPerTile);
	scene->getLayer(GScene::sceneLayers::lightmap)->addChild(node);
	lightmapNodes.insert_or_assign(id, node);
}

void graphics_context::addLightSource(SpriteID id, ConeLightArea light, SpaceVect pos, SpaceFloat angle)
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

void graphics_context::addLightSource(SpriteID id, SpriteLightArea light, SpaceVect pos, SpaceFloat angle)
{
	Sprite* s = Sprite::create(light.texName);

	s->setPosition(toCocos(pos) * app::pixelsPerTile);
	s->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	s->setScale(light.scale);
	s->setColor(toColor3B(light.color));
	scene->getLayer(GScene::sceneLayers::lightmap)->addChild(s);

	lightmapNodes.insert_or_assign(id, s);
}

void graphics_context::removeLightSource(SpriteID id)
{
	auto it = lightmapNodes.find(id);
	if (it != lightmapNodes.end()) {
		scene->getLayer(GScene::sceneLayers::lightmap)->removeChild(it->second);
		lightmapNodes.erase(it);
	}
	else {
		log1("unknown light source %u.", to_uint(id));
	}
	lightmapNoise.erase(id);
}

void graphics_context::setLightSourceNoise(SpriteID id, perlin_light_state noise)
{
	noise.crntAngle = noise.startAngle;
	lightmapNoise.insert_or_assign(id, noise);
}

void graphics_context::autoremoveLightSource(SpriteID id, float seconds)
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
	shared_ptr<sprite_properties> _sprite,
	SpaceFloat agentSize,
	GraphicsLayer sceneLayer,
	Vec2 pos
){
    if(!_sprite){
        log0("null sprite_properties");
        return;
    }

	sprite_properties sprite = *_sprite;

	float zoom = getSpriteZoom(_sprite, agentSize);
	if (sprite.size != make_pair(3, 4) && sprite.size != make_pair(4, 4)) {
		log2("Invalid agent animation size %d,%d.", sprite.size.first, sprite.size.second);
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
	label->runAction(damageIndicatorAction(startPos).generator());
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

void graphics_context::createConeShader(
		SpriteID id,
        GraphicsLayer layer,
		const Color4F color,
        float radius,
        const Vec2 center,
        SpaceFloat coneWidth,
        SpaceFloat initialAngleRad
) {
    ConeShader* shader = Node::ccCreate<ConeShader>(
        color,
        radius,
        center,
        coneWidth,
        initialAngleRad
    );
    shader->setContentSize(CCSize(radius, radius) * 2.0f * app::pixelsPerTile);

	graphicsNodes.insert_or_assign(id, shader);
	scene->getSpaceLayer()->positionAndAddNode(shader, to_int(layer), center, 1.0f);
}

void graphics_context::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(generator());
	}
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

			if (entry.fadeOut) {
				sprite->stopActionByTag(to_int(cocos_action_tag::object_fade));
				sprite->runAction(objectFadeOut(objectFadeOutTime, objectFadeOpacity).generator());
			}
			if (entry.fadeIn) {
				sprite->stopActionByTag(to_int(cocos_action_tag::object_fade));
				sprite->runAction(objectFadeOut(objectFadeInTime, 255).generator());
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
			Node* light = getOrDefault<SpriteID, Node*>(lightmapNodes, entry.lightID, nullptr);
			if (light) {
				light->setPosition(entry.pos);
				light->setRotation(entry.angle);
			}
		}
	}
}

void graphics_context::clearSubroomMask(unsigned int roomID)
{
	roomMasks.at(roomID)->setVisible(false);
}

void node_context::createSprite(
	string texture, GraphicsLayer layer,
	Vec2 pos, float zoom
){
	createNode(
		&graphics_context::createSprite,
		texture,
		layer,
		pos,
		zoom
	);
}

void node_context::createLightSource(
	shared_ptr<LightArea> light,
	SpaceVect pos,
	SpaceFloat angle
){
	createNode(
		&graphics_context::addPolyLightSource,
		light,
		pos,
		angle
	);
}

void node_context::runAction(GraphicsAction action)
{
	if(id != 0){
		addSceneAction(bind(
			&graphics_context::runSpriteAction,
			getGraphicsContext(),
			id,
			action.generator
		));
	}
}

void node_context::stopAction(cocos_action_tag tag)
{
	nodeMethod(&Node::stopActionByTag, to_int(tag));
}

void node_context::autoremoveLightSource(float seconds)
{
	addSceneAction(bind(
		&graphics_context::autoremoveLightSource,
		getGraphicsContext(),
		id,
		seconds
	));
}

void node_context::removeWithAction(ActionGeneratorType action)
{
	addSceneAction(bind(
		&graphics_context::removeSpriteWithAnimation,
		getGraphicsContext(),
		id,
		action
	));
}

void node_context::removeSprite()
{
	if(id != 0){
		addSceneAction(bind(
			&graphics_context::removeSprite,
			getGraphicsContext(),
			id
		));
	}
	id = 0;
}

void node_context::removeLightSource()
{
	addSceneAction(bind(
		&graphics_context::removeLightSource,
		getGraphicsContext(),
		id
	));
	id = 0;
}

void node_context::setVisible(bool v)
{
	nodeMethod(&Node::setVisible, v);
}

void node_context::setPos(SpaceVect p)
{
	nodeMethod<Node, const Vec2&>(&Node::setPosition, toCocos(p) * app::pixelsPerTile);
}

void node_context::setPosition(Vec2 p)
{
	nodeMethod<Node, const Vec2&>(&Node::setPosition, p);
}

void node_context::setAngle(SpaceFloat a)
{
	nodeMethod(&Node::setRotation, toCocosAngle(a));
}

void node_context::setRotation(float a)
{
	nodeMethod(&Node::setRotation, a);
}

void node_context::setScale(float zoom)
{
	nodeMethod<Node, float>(&Node::setScale, zoom);
}

void node_context::setColor(Color3B color)
{
	nodeMethod(&Node::setColor, color);
}

void node_context::setColor(Color4F color)
{
	nodeMethod(&Node::setColor, toColor3B(color));
}

void node_context::setOpacity(unsigned char op)
{
	nodeMethod(&Node::setOpacity, op);
}

void node_context::setTexture(string s)
{
	nodeMethod<Sprite, const string&>(&Sprite::setTexture, s);
}

void node_context::setShader(string s)
{
	nodeMethod(&Sprite::setShader, s);
}

void node_context::addSceneAction(zero_arity_function f)
{
	space->addSceneAction(f);
}

graphics_context* node_context::getGraphicsContext()
{
	return space->graphicsContext;
}

SpriteID node_context::getSpriteID()
{
	return space->graphicsContext->getSpriteID();
}

void node_context::clearDrawNode()
{
	nodeMethod(&DrawNode::clear);
}

void node_context::drawCircle(
	Vec2 center, float radius, float angle,
	unsigned int segments, const Color4F& color
){
	nodeMethod<
		DrawNode,
		const Vec2&, float, float,
		unsigned int, const Color4F&
	>(
		&DrawNode::drawSolidCircle,
		center,
		radius,
		angle,
		segments,
		color
	);
}

void node_context::drawRectangle(Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	nodeMethod(
		&DrawNode::drawSolidRect,
		lowerLeft,
		upperRight,
		color
	);
}
