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
#include "GSpace.hpp"
#include "scenes.h"

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

void SunArea::initializeGraphics(Layer* layer)
{
	space->getScene()->addLightSource(AmbientLightArea{
		getPos(),
		getDimensions(),
		Color3B(192, 192, 82),
		1.0f
	});
}

GraphicsLayer SunArea::sceneLayer() const{
	return GraphicsLayer::overhead;
}

AttributeMap SunArea::getAttributeEffect() {
	return {
		{ Attribute::sunDamage, 5.0  }
	};
}
