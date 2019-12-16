//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "GObject.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

spell_cost spell_cost::initialMP(float mp)
{
	return spell_cost{ mp, 0.0f, 0.0f, 0.0f };
}

spell_cost spell_cost::initialStamina(float stamina)
{
	return spell_cost{ 0.0f, stamina, 0.0f, 0.0f };
}

spell_cost spell_cost::ongoingMP(float mp)
{
	return spell_cost{ 0.0f, 0.0f, mp, 0.0f };
}

Spell::Spell(GObject* caster, SpaceFloat length, SpaceFloat updateInterval, spell_cost cost) :
	caster(caster),
	length(length),
	updateInterval(updateInterval),
	_cost(cost)
{}

Spell::~Spell() {}

bool Spell::isActive() const {
	return crntState == state::active;
}

GSpace* Spell::getSpace() const {
	return caster->space;
}

void Spell::runUpdate()
{
	timerIncrement(t);
	timerIncrement(lastUpdate);

	if (updateInterval >= 0.0 && lastUpdate >= updateInterval) {
		update();
		lastUpdate -= updateInterval;
	}

	if (length > 0.0 && t >= length) {
		runEnd();
	}
}

void Spell::runEnd()
{
	crntState = state::ending;
	end();
	caster->crntSpell.reset();
	crntState = state::expired;
}

shared_ptr<SpellDesc> Spell::getDescriptorByName(const string& name)
{
	auto it = spellDescriptors.find(name);

	if (it != spellDescriptors.end())
		return it->second;
	else
		return nullptr;
}

