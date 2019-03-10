//
//  EnemySpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

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
const float TorchDarkness::effectTime = 1.0f;

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

	for (auto it = torches.begin(); it != torches.end(); )
	{
		pair<Torch*, float> entry = *it;

		if (crntTorches.find(it->first) == crntTorches.end())
		{
			it = torches.erase(it);
		}
		else
		{
			timerDecrement(it->second);

			if (it->second <= 0.0f)
			{
				it->first->setActive(false);
				it = torches.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	for (Torch* crnt : crntTorches)
	{
		if (crnt->getActive() && torches.find(crnt) == torches.end())
		{
			torches.insert_or_assign(crnt, effectTime);
		}
	}
}
