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
#include "Player.hpp"
#include "scenes.h"

EffectArea::EffectArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
MapObjForwarding(AreaSensor),
RegisterUpdate<EffectArea>(this)
{
}

void EffectArea::update()
{
	if (player.isValid())
	{
		player.get()->applyAttributeEffects(AttributeSystem::scale(getAttributeEffect(), App::secondsPerFrame));
	}
}

SunArea::SunArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(EffectArea)
{
}

void SunArea::initializeGraphics()
{
	space->addLightSource(AmbientLightArea{
		getPos(),
		getDimensions(),
		toColor4F(Color3B(192, 192, 82))
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
