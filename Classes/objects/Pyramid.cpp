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
#include "macros.h"
#include "Pyramid.hpp"
#include "SpaceLayer.h"

const SpaceFloat Pyramid::coneLength = 4.0;
const SpaceFloat Pyramid::coneAngle = float_pi / 2.0;
const unsigned int Pyramid::coneSegments = 128;
const Color4F Pyramid::coneColor = Color4F(0.75f, 0.6f, 0.4f, 0.7f);

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
}

void Pyramid::init()
{
	initStateMachine(fsm);
}

void Pyramid::update()
{
	fsm.update();

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
	drawNode->clear();
	SpaceFloat a = getAngle();
	
	drawNode->drawSolidCone( 
		Vec2::ZERO,
		coneLength * App::pixelsPerTile,
		a - coneAngle / 2.0 + float_pi / 2.0,
		a + coneAngle / 2.0 + float_pi / 2.0,
		coneSegments,
		coneColor
	);
}

void Pyramid::setAngle(SpaceFloat a)
{
	GObject::setAngle(a);

	redrawLightCone();
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

void Pyramid::initializeGraphics(SpaceLayer* layer)
{
	base = Sprite::create(imageSpritePath());
	drawNode = DrawNode::create();

	layer->getLayer(GraphicsLayer::ground)->positionAndAddNode(
		base,
		1,
		getInitialCenterPix(),
		1.0f
	);

	layer->getLayer(GraphicsLayer::overhead)->positionAndAddNode(
		drawNode,
		1,
		getInitialCenterPix(),
		1.0f
	);

	redrawLightCone();
}

void Pyramid::initStateMachine(ai::StateMachine& sm) {
	addThread(std::make_shared<ai::QuadDirectionLookAround>(boost::rational<int>(3,2),true));
}
