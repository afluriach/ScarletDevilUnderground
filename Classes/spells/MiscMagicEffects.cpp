//
//  MiscMagicEffects.cpp
//  Koumachika
//
//  Created by Toni on 3/12/19.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "MiscMagicEffects.hpp"
#include "SpellUtil.hpp"
#include "TeleportPad.hpp"

Teleport::Teleport(effect_params params) :
	MagicEffect(params),
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
		TeleportPad* crnt = ref.getAs<TeleportPad>();
		if (crnt && !crnt->isObstructed()) {
			log("%s teleported to %s.", target->toString(), crnt->toString());
			target->teleport(ref.get()->getPos());
			crnt->setTeleportActive(true);
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
		toUse.getAs<TeleportPad>()->setTeleportActive(false);
	}
}
