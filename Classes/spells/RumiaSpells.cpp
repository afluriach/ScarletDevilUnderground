//
//  RumiaSpells.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "RumiaSpells.hpp"
#include "value_map.hpp"

const string DarknessSignDemarcation::name = "DarknessSignDemarcation";
const string DarknessSignDemarcation::description = "";

const SpaceFloat DarknessSignDemarcation::betweenBurstDelay = 1.5;
const SpaceFloat DarknessSignDemarcation::burstInterval = 0.3;
const SpaceFloat DarknessSignDemarcation::launchDist = 1.5;
const SpaceFloat DarknessSignDemarcation::legSpacing = float_pi / 6.0;
const int DarknessSignDemarcation::burstCount = 4;
const int DarknessSignDemarcation::legCount = 8;
const int DarknessSignDemarcation::bulletsPerLeg = 3;

DarknessSignDemarcation::DarknessSignDemarcation(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id) :
	Spell(caster, desc, id, spell_params{ -1.0, 0.0 })
{
}

void DarknessSignDemarcation::update()
{
	timerDecrement(timer);

	if (timer <= 0.0)
	{
		generate();

		++crntBurst;

		if (crntBurst >= burstCount) {
			timer = betweenBurstDelay;
			crntBurst = 0;
			oddWave = !oddWave;
		}
		else {
			timer = burstInterval;
		}
	}
}

void DarknessSignDemarcation::generate()
{
	SpaceVect origin = caster->getPos();
	SpaceFloat radialStep = float_pi * 2.0 / legCount;

	for_irange(i, 0, legCount)
	{
		SpaceFloat legAngle = i * radialStep + to_int(oddWave)*radialStep*0.5;
		SpaceFloat legWidth = radialStep - legSpacing;
		SpaceFloat bulletRadialStep = legWidth / bulletsPerLeg;

		for_irange(j, 0, bulletsPerLeg)
		{
			SpaceFloat bulletAngle = legAngle - 0.5*legWidth + bulletRadialStep*j;
			SpaceVect pos = origin + SpaceVect::ray(launchDist, bulletAngle);

			gobject_ref bullet = launchBullet(
				app::getBullet("rumiaDemarcationBullet"),
				pos,
				bulletAngle
			);
		}
	}
}

const string DarknessSignDemarcation2::name = "DarknessSignDemarcation2";
const string DarknessSignDemarcation2::description = "";

const SpaceFloat DarknessSignDemarcation2::betweenBurstDelay = 5.0;
const SpaceFloat DarknessSignDemarcation2::burstInterval = 2.0 / 3.0;
const SpaceFloat DarknessSignDemarcation2::launchDist = 1.5;
const SpaceFloat DarknessSignDemarcation2::angleSkew = float_pi * 4.0 / 3.0;
const int DarknessSignDemarcation2::burstCount = 4;
const int DarknessSignDemarcation2::bulletsPerBurst = 24;

DarknessSignDemarcation2::DarknessSignDemarcation2(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id) :
	Spell(caster, desc, id, spell_params{ -1.0, 0.0 }),
	angularSpeed(angleSkew)
{
}

void DarknessSignDemarcation2::init()
{
	generate();
}

void DarknessSignDemarcation2::update()
{
	SpaceVect deltaPos = caster->getDeltaPos();
	if (!deltaPos.isZero())
	{
		auto it = bullets.begin();
		while (it != bullets.end()) {
			if (!it->isValid() && !it->isFuture()) {
				it = bullets.erase(it);
			}
			else {
				if (!it->isFuture()) {
					GObject* bullet = it->get();
					bullet->setPos(bullet->getPos() + deltaPos);
				}
				++it;
			}
		}
	}
}

void DarknessSignDemarcation2::end()
{
	for (auto ref : bullets) {
		if (ref.isFuture() || ref.isValid()) {
			getSpace()->removeObject(ref);
		}
	}
}

void DarknessSignDemarcation2::generate()
{
	SpaceVect origin = caster->getPos();
	SpaceFloat radialStep = float_pi * 2.0 / bulletsPerBurst;
	auto props = app::getBullet("rumiaDemarcationBullet2");

	for_irange(i, 0, bulletsPerBurst)
	{
		SpaceVect pos = SpaceVect::ray(launchDist, i*radialStep);
		SpaceFloat angle = radialStep * i ;
		SpaceFloat angularVel = (i % 2 ? 1.0 : -1.0)*angularSpeed;

		gobject_ref bullet = launchBullet(
			props,
			pos,
			angle,
			angularVel
		);
		bullets.insert(bullet);

		parametric_space_function f = [props, angle, angularVel](SpaceFloat t)->SpaceVect {
			return SpaceVect::ray(props->speed, angle + t*angularVel);
		};
		caster->makeInitMessage(&GObject::setParametricMove, bullet, f, parametric_type::velocity);
	}
}

const string NightSignPinwheel::name = "NightSignPinwheel";
const string NightSignPinwheel::description = "";

const int NightSignPinwheel::legCount = 12;
const int NightSignPinwheel::bulletsPerLegCount = 4;
const SpaceFloat NightSignPinwheel::launchDelay = 2.0;
const SpaceFloat NightSignPinwheel::refreshRatio = 0.25;
const SpaceFloat NightSignPinwheel::legLength = 3.0;
const SpaceFloat NightSignPinwheel::legStartDist = 1.0;
const SpaceFloat NightSignPinwheel::legAngleSkew = float_pi / NightSignPinwheel::legCount;

NightSignPinwheel::NightSignPinwheel(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id) :
	Spell(caster, desc, id, spell_params{ -1.0, 0.0 })
{
}

void NightSignPinwheel::update()
{
	if (bulletsExistingRatio() < refreshRatio)
	{
		generate();
		waitingToLaunch = true;
		timer = launchDelay;
	}

	if (waitingToLaunch)
	{
		timerDecrement(timer);
		if (timer <= 0.0) {
			launch();
			waitingToLaunch = false;
		}
	}
}

void NightSignPinwheel::end()
{
	removeBullets();
}

SpaceFloat NightSignPinwheel::bulletsExistingRatio()
{
	int count = 0;

	for (gobject_ref bullet : bullets) {
		if (bullet.isValid())
			++count;
	}

	return count / (legCount * bulletsPerLegCount);
}

void NightSignPinwheel::generate()
{
	for_irange(i, 0, legCount) {
		generateLeg(float_pi * 2.0 * i / legCount);
	}
}

void NightSignPinwheel::generateLeg(SpaceFloat angle)
{
	SpaceFloat posStep = legLength / bulletsPerLegCount;
	SpaceFloat angleStep = legAngleSkew / bulletsPerLegCount;

	for_irange(i, 0, bulletsPerLegCount) {
		SpaceFloat d = legStartDist + posStep * i;
		SpaceFloat a = angle + angleStep * i;
		SpaceVect pos = SpaceVect::ray(d, a);

		gobject_ref bullet = launchBullet(
			app::getBullet("rumiaPinwheelBullet"),
			pos,
			a
		);
		bullets.insert(bullet);
	}
}

void NightSignPinwheel::launch()
{
	for (gobject_ref bullet : bullets) {
		if(bullet.isValid())
			bullet.get()->launch();
	}
}

void NightSignPinwheel::removeBullets()
{
	for (gobject_ref bullet : bullets)
	{
		if (bullet.isValid()) {
			getSpace()->removeObject(bullet);
		}
	}
	bullets.clear();
}
