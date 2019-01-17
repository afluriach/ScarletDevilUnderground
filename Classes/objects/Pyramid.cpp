//
//  Pyramid.cpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "GSpace.hpp"
#include "macros.h"
#include "Pyramid.hpp"
#include "scenes.h"
#include "value_map.hpp"

const SpaceFloat Pyramid::coneLength = 4.0;
const SpaceFloat Pyramid::coneAngle = float_pi / 2.0;
const unsigned int Pyramid::coneSegments = 32;
const Color4F Pyramid::coneColor = Color4F(0.75f, 0.6f, 0.4f, 0.7f);
const Color4F Pyramid::coneActiveColor = Color4F(0.9f, 0.75f, 0.5f, 0.7f);

const AttributeMap Pyramid::lightConeEffect = {
	{Attribute::sunDamage, 5.0f }
};


Pyramid::Pyramid(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
RegisterInit<Pyramid>(this),
RegisterUpdate<Pyramid>(this),
RectangleMapBody(args),
StateMachineObject(args)
{
	angular_speed = getFloatOrDefault(args, "angular_speed", 0.0f) / 180.0 * float_pi;

	string discrete = getStringOrDefault(args, "discrete_look", "");
	if (!discrete.empty()) {
		discrete_look = boost::lexical_cast<boost::rational<int>>(discrete);
	}
}

void Pyramid::init()
{
	initStateMachine(fsm);
}

void Pyramid::update()
{
	fsm.update();
	redrawLightCone();

	for (object_ref<Agent> agent_ref : targets)
	{
		Agent* agent = agent_ref.get();
		if (agent) {
			agent->applyAttributeEffects(AttributeSystem::scale(lightConeEffect, App::secondsPerFrame));
		}
	}
}

void Pyramid::redrawLightCone()
{
	space->getScene()->clearDrawNode(drawNodeID);
	SpaceFloat a = getAngle();
	
	space->getScene()->drawSolidCone(
		drawNodeID,
		Vec2::ZERO,
		coneLength * App::pixelsPerTile,
		a - coneAngle / 2.0,
		a + coneAngle / 2.0,
		coneSegments,
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
	spriteID = space->getScene()->createSprite(imageSpritePath(), GraphicsLayer::ground, getInitialCenterPix(), 1.0f);
	drawNodeID = space->getScene()->createDrawNode(GraphicsLayer::overhead, getInitialCenterPix(), 1.0f);

	redrawLightCone();
}

void Pyramid::initStateMachine(ai::StateMachine& sm) {
	if(angular_speed != 0.0)
		addThread(std::make_shared<ai::LookAround>(angular_speed));
	else if(discrete_look != 0)
		addThread(std::make_shared<ai::QuadDirectionLookAround>(abs(discrete_look), discrete_look > 0));
}
