//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"

spell_cost spell_cost::none()
{
	return spell_cost{ 0.0f,0.0f,0.0f,0.0f };
}

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

spell_params::spell_params(
	SpaceFloat length,
	SpaceFloat updateInterval,
	spell_cost cost
) :
	length(length),
	updateInterval(updateInterval),
	cost(cost)
{
}

Spell::Spell(GObject* caster, const SpellDesc* desc, unsigned int id, spell_params params) :
	caster(caster),
	descriptor(desc),
	id(id),
	length(params.length),
	updateInterval(params.updateInterval),
	_cost(params.cost)
{}

Spell::~Spell() {}

GSpace* Spell::getSpace() const {
	return caster->space;
}

bullet_attributes Spell::getBulletAttributes(local_shared_ptr<bullet_properties> props) const
{
	auto result = caster->getBulletAttributes(props);
	result.sourceSpell = id;
	return result;
}

gobject_ref Spell::spawnBullet(
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceVect velocity,
	SpaceFloat angle,
	SpaceFloat angularVelocity
) {
	return caster->_spawnBullet(
		getBulletAttributes(props),
		props,
		displacement,
		velocity,
		angle,
		angularVelocity
	);
}

gobject_ref Spell::launchBullet(
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceFloat angle,
	SpaceFloat angularVelocity,
	bool obstacleCheck
) {
	return caster->_launchBullet(
		getBulletAttributes(props),
		props,
		displacement,
		angle,
		angularVelocity,
		obstacleCheck
	);
}

unsigned int Spell::getID() const {
	return id;
}

string Spell::getName() const {
	return descriptor->getName();
}

spell_cost Spell::getCost() const {
	return descriptor->getCost();
}

SpaceFloat Spell::getTime() const {
	return t;
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
		stop();
	}
}

void Spell::stop()
{
	getSpace()->spellSystem->stopSpell(id);
}

const SpellDesc* Spell::getDescriptorByName(const string& name)
{
	auto it = spellDescriptors.find(name);

	if (it != spellDescriptors.end())
		return it->second;
	else
		return app::getSpell(name);
}
