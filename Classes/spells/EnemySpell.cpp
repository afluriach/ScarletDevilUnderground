//
//  EnemySpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "EnemySpell.hpp"
#include "GSpace.hpp"
#include "TeleportPad.hpp"
#include "Torch.hpp"

const string Teleport::name = "Teleport";
const string Teleport::description = "";

Teleport::Teleport(GObject* caster) :
	Spell(caster),
	targets(caster->space->getObjectsByTypeAs<TeleportPad>())
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
			log("%s teleported to %s.", caster->getName().c_str(), ref.get()->getName().c_str());
			caster->teleport(ref.get()->getPos());
			ref.get()->setTeleportActive(true);
			success = true;
			toUse = ref;
			break;
		}
	}

	if (success) {
		active = false;
	}
}

void Teleport::end()
{
	if (toUse.isValid()) {
		toUse.get()->setTeleportActive(false);
	}
}

const SpaceFloat TorchDarkness::radius = 2.5f;
const float TorchDarkness::effectMagnitude = 0.2f;

TorchDarkness::TorchDarkness(GObject* caster) :
	Spell(caster)
{}

void TorchDarkness::update()
{
	unordered_set<Torch*> crntTorches = caster->space->radiusQueryByType<Torch>(
		caster,
		caster->getPos(),
		radius,
		GType::environment,
		PhysicsLayers::all
	);

	for (Torch* crnt : crntTorches)
	{
		if (crnt->getActive())
		{
			crnt->applyDarkness(effectMagnitude * App::secondsPerFrame);
		}
	}
}
