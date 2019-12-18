//
//  MiscMagicEffects.cpp
//  Koumachika
//
//  Created by Toni on 3/12/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "SpellUtil.hpp"
#include "TeleportPad.hpp"

DrainFromMovement::DrainFromMovement(Agent* agent, Attribute attr, float unitsPerMeter) :
	MagicEffect(agent, 0.0f, 0.0f, enum_bitfield2(flags, indefinite, active)),
	agent(agent),
	attr(attr)
{
	_ratio = -1.0f * app::params.secondsPerFrame * unitsPerMeter;
}

void DrainFromMovement::update()
{
	agent->modifyAttribute(attr, _ratio * agent->getAttribute(Attribute::currentSpeed) );
}

Teleport::Teleport(GObject* target) :
	MagicEffect(target, 0.0f, 0.0f, enum_bitfield2(flags, indefinite, active)),
	targets(getSpace()->getObjectsByTypeAs<TeleportPad>())
{
}

void Teleport::init()
{
}

void Teleport::update()
{
	bool success = false;

	for (auto ref : targets)
	{
		if (ref.isValid() && !ref.get()->isObstructed()) {
			log("%s teleported to %s.", agent->getName().c_str(), ref.get()->getName().c_str());
			agent->teleport(ref.get()->getPos());
			ref.get()->setTeleportActive(true);
			success = true;
			toUse = ref;
			break;
		}
	}

	if (success) {
		remove();
	}
}

void Teleport::end()
{
	if (toUse.isValid()) {
		toUse.get()->setTeleportActive(false);
	}
}
