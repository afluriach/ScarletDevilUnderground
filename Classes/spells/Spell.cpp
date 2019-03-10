//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

Spell::Spell(GObject* caster) :
	caster(caster)
{}

Spell::~Spell() {}

bool Spell::isActive() const {
	return active;
}

shared_ptr<SpellDesc> Spell::getDescriptorByName(const string& name)
{
	auto it = spellDescriptors.find(name);

	if (it != spellDescriptors.end())
		return it->second;
	else
		return nullptr;
}

void PeriodicSpell::update() {

	float _interval = interval();

	if (_interval == 0.0f)
	{
		runPeriodic();
	}

	else
	{
		timeSince += App::secondsPerFrame;

		if (timeSince >= _interval) {
			timeSince -= _interval;
			runPeriodic();
		}
	}
}
