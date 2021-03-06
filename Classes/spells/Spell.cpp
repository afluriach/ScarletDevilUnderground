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
	return spell_cost( 0.0f, 0.0f, 0.0f, 0.0f );
}

spell_cost spell_cost::initialMP(float mp)
{
	return spell_cost( mp, 0.0f, 0.0f, 0.0f );
}

spell_cost spell_cost::initialStamina(float stamina)
{
	return spell_cost( 0.0f, stamina, 0.0f, 0.0f );
}

spell_cost spell_cost::ongoingMP(float mp)
{
	return spell_cost( 0.0f, 0.0f, mp, 0.0f );
}

Spell::Spell(GObject* caster, const SpellDesc* desc, unsigned int id) :
	caster(caster),
	descriptor(desc),
	id(id)
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

vector<gobject_ref> Spell::spawnBulletRadius(
	local_shared_ptr<bullet_properties> props,
	SpaceFloat displacement,
	int count
) {
	vector<gobject_ref> result;
	result.reserve(count);

	for_irange(i, 0, count)
	{
		SpaceFloat angle = (1.0 * i / count) * (float_pi * 2.0);

		result.push_back(spawnBullet(
			props,
			SpaceVect::ray(displacement, angle),
			SpaceVect::zero,
			angle,
			0.0
		));
	}

	return result;
}

void Spell::bulletCircle(
	const vector<gobject_ref>& bullets,
	SpaceFloat distance,
	SpaceFloat angularPos
) {
	if (bullets.empty()) return;

	SpaceVect pos = caster->getPos();
	SpaceFloat angleStep = float_pi * 2.0 / bullets.size();

	for_irange(i, 0, bullets.size())
	{
		SpaceFloat angle = i * angleStep + angularPos;
		SpaceVect disp = SpaceVect::ray(distance, angle);

		if (bullets[i].isValid()) {
			bullets[i].get()->setPos(pos + disp);
			bullets[i].get()->setAngle(angle);
		}
	}
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

	auto updateInterval = descriptor->params.updateInterval;
	auto length = descriptor->params.length;

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
