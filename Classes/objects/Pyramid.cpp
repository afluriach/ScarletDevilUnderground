//
//  Pyramid.cpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "Pyramid.hpp"
#include "value_map.hpp"

const SpaceFloat Pyramid::coneLength = 4.0;
const SpaceFloat Pyramid::coneWidth = float_pi / 2.0;
const unsigned int Pyramid::coneSegments = 32;
const Color4F Pyramid::coneColor = Color4F(0.75f, 0.6f, 0.4f, 0.7f);
const Color4F Pyramid::coneActiveColor = Color4F(0.9f, 0.75f, 0.5f, 0.7f);

const DamageInfo Pyramid::lightConeEffect = DamageInfo{ 5.0f, Attribute::sunDamage, DamageType::effectArea };

Pyramid::Pyramid(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject)
{
	angular_speed = toRads(getFloatOrDefault(args, "angular_speed", 0.0f));

	auto it = args.find("direction");
	if (it != args.end()) {
		Direction dir = stringToDirection(it->second.asString());
		if (dir != Direction::none)
			setInitialAngle(dirToPhysicsAngle(dir));
	}
}

void Pyramid::update()
{
	GObject::update();
	RadarObject::_update();

	setAngle(getAngle() + angular_speed * app::params.secondsPerFrame);

	redrawLightCone();

	for (object_ref<Agent> agent_ref : targets)
	{
		Agent* agent = agent_ref.get();
		if (agent) {
			agent->hit(lightConeEffect);
		}
	}
}

void Pyramid::redrawLightCone()
{	
	SpaceFloat angle = getAngle();
	space->addGraphicsAction(
		&graphics_context::setLightSourceColor,
		lightID,
		targets.empty() ? coneColor : coneActiveColor
	);
}

PhysicsLayers Pyramid::getLayers() const{
    return PhysicsLayers::ground;
}

void Pyramid::onDetect(GObject* other)
{
	Agent* agent = dynamic_cast<Agent*>(other);

	if (agent)
		targets.insert(agent);
}

void Pyramid::onEndDetect(GObject* other)
{
	Agent* agent = dynamic_cast<Agent*>(other);

	if (agent)
		targets.erase(agent);
}

void Pyramid::initializeGraphics()
{
	SpaceFloat a = getAngle();
	spriteID = space->createSprite(
		&graphics_context::createSprite,
		imageSpritePath(),
		GraphicsLayer::ground,
		getInitialCenterPix(),
		1.0f
	);
	lightID = space->addLightSource(ConeLightArea{
		getPos(),
		coneLength,
		coneWidth,
		coneColor
	});
	space->addGraphicsAction(&graphics_context::setLightSourceAngle, lightID, getAngle());
}
