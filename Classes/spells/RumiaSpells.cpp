//
//  RumiaSpells.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "RumiaSpells.hpp"
#include "util.h"
#include "value_map.hpp"

const string DarknessSignDemarcation::name = "DarknessSignDemarcation";
const string DarknessSignDemarcation::description = "";
const float DarknessSignDemarcation::cost = 0.0f;

const SpaceFloat DarknessSignDemarcation::betweenBurstDelay = 1.5;
const SpaceFloat DarknessSignDemarcation::burstInterval = 0.3;
const SpaceFloat DarknessSignDemarcation::launchDist = 1.5;
const SpaceFloat DarknessSignDemarcation::angleSkew = float_pi / DarknessSignDemarcation::bulletsPerBurst;
const int DarknessSignDemarcation::burstCount = 4;
const int DarknessSignDemarcation::bulletsPerBurst = 24;

DarknessSignDemarcation::DarknessSignDemarcation(GObject* caster) :
	Spell(caster)
{
}

void DarknessSignDemarcation::update()
{
	timerDecrement(timer);

	if (timer <= 0.0)
	{
		generate();

		++crntBurst;

		timer = crntBurst < burstCount ? burstInterval : betweenBurstDelay;
		crntBurst = crntBurst < burstCount ? crntBurst : 0;
	}
}

void DarknessSignDemarcation::generate()
{
	SpaceVect origin = caster->getPos();
	SpaceFloat radialStep = float_pi * 2.0 / bulletsPerBurst;

	for_irange(i, 0, bulletsPerBurst)
	{
		SpaceVect pos = origin + SpaceVect::ray(launchDist, i*radialStep);
		SpaceFloat angle = radialStep * i + (i % 2 ? 1.0 : -1.0)*angleSkew;

		gobject_ref bullet = getCasterAs<Agent>()->bulletImplCheckSpawn<EnemyBulletImpl>(
			pos,
			angle,
			&EnemyBulletImpl::rumiaDemarcationBullet
		);
	}
}

const string DarknessSignDemarcation2::name = "DarknessSignDemarcation2";
const string DarknessSignDemarcation2::description = "";
const float DarknessSignDemarcation2::cost = 0.0f;

const SpaceFloat DarknessSignDemarcation2::betweenBurstDelay = 5.0;
const SpaceFloat DarknessSignDemarcation2::burstInterval = 2.0 / 3.0;
const SpaceFloat DarknessSignDemarcation2::launchDist = 1.5;
const SpaceFloat DarknessSignDemarcation2::angleSkew = float_pi / 1.5;
const int DarknessSignDemarcation2::burstCount = 4;
const int DarknessSignDemarcation2::bulletsPerBurst = 24;

DarknessSignDemarcation2::DarknessSignDemarcation2(GObject* caster) :
	Spell(caster)
{
}

void DarknessSignDemarcation2::update()
{
	timerDecrement(timer);

	if (timer <= 0.0)
	{
		generate();

		++crntBurst;

		timer = crntBurst < burstCount ? burstInterval : betweenBurstDelay;
		crntBurst = crntBurst < burstCount ? crntBurst : 0;
	}
}

void DarknessSignDemarcation2::generate()
{
	SpaceVect origin = caster->getPos();
	SpaceFloat radialStep = float_pi * 2.0 / bulletsPerBurst;

	for_irange(i, 0, bulletsPerBurst)
	{
		SpaceVect pos = origin + SpaceVect::ray(launchDist, i*radialStep);
		SpaceFloat angle = radialStep * i ;

		gobject_ref bullet = getCasterAs<Agent>()->bulletCheckSpawn<RumiaDemarcation2Bullet>(
			pos,
			angle,
			(i % 2 ? 1.0 : -1.0)*angleSkew,
			betweenBurstDelay * 1.5
		);
	}
}

const string NightSignPinwheel::name = "NightSignPinwheel";
const string NightSignPinwheel::description = "";
const float NightSignPinwheel::cost = 0.0f;

const int NightSignPinwheel::legCount = 12;
const int NightSignPinwheel::bulletsPerLegCount = 4;
const SpaceFloat NightSignPinwheel::launchDelay = 2.0;
const SpaceFloat NightSignPinwheel::refreshRatio = 0.25;
const SpaceFloat NightSignPinwheel::legLength = 3.0;
const SpaceFloat NightSignPinwheel::legStartDist = 1.0;
const SpaceFloat NightSignPinwheel::legAngleSkew = float_pi / NightSignPinwheel::legCount;

NightSignPinwheel::NightSignPinwheel(GObject* caster) :
	Spell(caster)
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
	SpaceVect origin = caster->getPos();
	SpaceFloat posStep = legLength / bulletsPerLegCount;
	SpaceFloat angleStep = legAngleSkew / bulletsPerLegCount;

	for_irange(i, 0, bulletsPerLegCount) {
		SpaceFloat d = legStartDist + posStep * i;
		SpaceFloat a = angle + angleStep * i;
		SpaceVect pos = origin + SpaceVect::ray(d, a);

		gobject_ref bullet = getCasterAs<Agent>()->bulletImplCheckSpawn<EnemyBulletImpl>(
			pos,
			a,
			&EnemyBulletImpl::rumiaPinwheelBullet
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
			caster->space->removeObject(bullet);
		}
	}
	bullets.clear();
}
