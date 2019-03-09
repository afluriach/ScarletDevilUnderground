//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "TeleportPad.hpp"
#include "Torch.hpp"
#include "util.h"
#include "value_map.hpp"

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

const string StarlightTyphoon::name = "StarlightTyphoon";
const string StarlightTyphoon::description = "";

const SpaceFloat StarlightTyphoon::speed = 6.0;
const SpaceFloat StarlightTyphoon::width = float_pi / 4.0;
const SpaceFloat StarlightTyphoon::radius = 0.2;
const SpaceFloat StarlightTyphoon::duration = 1.0;
const SpaceFloat StarlightTyphoon::offset = 0.7;

const unsigned int StarlightTyphoon::count = 30;

StarlightTyphoon::StarlightTyphoon(GObject* caster):
Spell(caster)
{    
	angle = caster->getAngle();

    shotsPerFrame = count / duration * App::secondsPerFrame;
}


void StarlightTyphoon::init()
{
}
void StarlightTyphoon::fire()
{
    //The angle variation, will be added to the base direction.
    SpaceFloat arcPos = App::getRandomFloat(-width, width) + caster->getAngle();
	SpaceFloat crntSpeed = App::getRandomFloat(speed*0.5, speed*1.5);
	SpaceFloat crntRadius = App::getRandomFloat(radius*0.7, radius*1.3);
    
    SpaceVect pos = caster->getPos() + SpaceVect::ray(offset, angle);
    
	caster->space->createObject(GObject::make_object_factory<StarBullet>(
		pos,
		arcPos,
		dynamic_cast<Agent*>(caster),
		crntSpeed,
		radius,
		StarBullet::colors[App::getRandomInt(0, StarBullet::colors.size() - 1)]
	));
}
void StarlightTyphoon::update()
{
    accumulator += shotsPerFrame;
    elapsed += App::secondsPerFrame;
    
    while(accumulator >= 1){
        fire();
        accumulator -= 1;
    }
    
    if(elapsed > duration){
        caster->stopSpell();
    }
}
void StarlightTyphoon::end()
{
}

//void PuppetryDoll::init()
//{
//}

const string IllusionDial::name = "IllusionDial";
const string IllusionDial::description = "";

const int IllusionDial::count = 16;

const float IllusionDial::radius = 2.5f;
const float IllusionDial::arc_start = 0.0f;
const float IllusionDial::arc_end = float_pi * 2.0f;
const float IllusionDial::arc_width = arc_end - arc_start;
const float IllusionDial::arc_spacing = arc_width / (count-1);
const float IllusionDial::angular_speed = float_pi * 2.0f / 3.0f;

const float IllusionDial::max_angle_margin = float_pi / 12.0f;
const float IllusionDial::min_fire_interval = 1.0f / 3.0f;

IllusionDial::IllusionDial(GObject* caster) :
Spell(caster),
bullets(count),
launch_flags(count, false)
{}

void IllusionDial::init()
{
    //refactor utility function for this
    //getFloatOrDefault(args, "radius", radius);
    
    for_irange(i,0,count)
    {
		bullets[i] = caster->space->createObject(GObject::make_object_factory<IllusionDialDagger>(
			dynamic_cast<Agent*>(caster),
			caster->getPos() + SpaceVect::ray(radius, arc_start + i * arc_spacing),
			i % 2 ? angular_speed : -angular_speed
		));
    }
}

void IllusionDial::update()
{
	++framesSinceLastFire;

	if (framesSinceLastFire*App::secondsPerFrame >= min_fire_interval)
	{

		int best = -1;
		SpaceFloat best_angle = float_pi;
		bool allBulletsConsumed = true;

		for_irange(i, 0, count)
		{
			if (bullets[i].isValid() && !launch_flags[i]) {
				allBulletsConsumed = false;

				SpaceFloat crnt = bullets[i].get()->targetViewAngle();

				if (!isinf(crnt) && abs(crnt) < best_angle)
					best = i;
				best_angle = abs(crnt);
			}
		}

		if (best != -1 && best_angle < max_angle_margin)
		{
			bullets[best].get()->launch();
			launch_flags[best] = true;
			framesSinceLastFire = 0;
		}
		else if (allBulletsConsumed)
		{
			//Deactivate spell if all bullets are consumed.
			active = false;
		}
	}
}

void IllusionDial::end()
{
	for (auto ref : bullets) {
		if (ref.isValid()) {
			caster->space->removeObject(ref.get());
		}
	}
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

const SpaceFloat TorchDarkness::radius = 2.5f;
const float TorchDarkness::effectTime = 1.0f;

TorchDarkness::TorchDarkness(GObject* caster) :
	Spell(caster)
{};

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

const string DarknessSignDemarcation::name = "DarknessSignDemarcation";
const string DarknessSignDemarcation::description = "";

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
			(i % 2 ? 1.0 : -1.0)*angleSkew
		);
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
			caster->space->removeObject(bullet.get());
		}
	}
	bullets.clear();
}
