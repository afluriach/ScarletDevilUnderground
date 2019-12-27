//
//  MiscMagicEffects.cpp
//  Koumachika
//
//  Created by Toni on 3/12/19.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "MiscMagicEffects.hpp"
#include "SpellUtil.hpp"
#include "TeleportPad.hpp"

Teleport::Teleport(effect_params params, float magnitude, float length) :
	MagicEffect(params, magnitude, length),
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
			log("%s teleported to %s.", target->getName().c_str(), ref.get()->getName().c_str());
			target->teleport(ref.get()->getPos());
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
