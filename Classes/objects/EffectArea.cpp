//
//  EffectArea.cpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "EffectArea.hpp"
#include "SpaceLayer.h"

EffectArea::EffectArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
RegisterUpdate<EffectArea>(this),
RectangleMapBody(args)
{
}

void EffectArea::update()
{
	for (auto ref : contacts)
	{
		Agent* agent = ref.get();

		if (agent) {
			agent->applyAttributeEffects(AttributeSystem::scale(getAttributeEffect(), App::secondsPerFrame));
		}
	}
}

void EffectArea::onContact(Agent* agent)
{
	contacts.insert(agent);
}

void EffectArea::onEndContact(Agent* agent)
{
	contacts.erase(agent);
}

PhysicsLayers EffectArea::getLayers() const{
    return PhysicsLayers::ground;
}

SunArea::SunArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(EffectArea)
{
}

void SunArea::initializeGraphics(SpaceLayer* layer)
{
	DrawNode* dn = DrawNode::create();
	SpaceVect dim = getDimensions();
	dn->drawSolidRect(
		Vec2(-dim.x, -dim.y) * App::pixelsPerTile / 2.0,
		Vec2(dim.x, dim.y) * App::pixelsPerTile / 2.0,
		Color4F(.75f, .75f, .33f, .25f)
	);

	layer->getLayer(sceneLayer())->positionAndAddNode(dn, 1, getInitialCenterPix(), 1.0f);
	
	sprite = dn;	
}

GraphicsLayer SunArea::sceneLayer() const{
	return GraphicsLayer::overhead;
}

AttributeMap SunArea::getAttributeEffect() {
	return {
		{ Attribute::sunDamage, 5.0  }
	};
}
