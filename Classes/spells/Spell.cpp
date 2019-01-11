//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "TeleportPad.hpp"
#include "util.h"
#include "value_map.hpp"

Spell::Spell(GObject* caster, const ValueMap& args, SpellDesc* descriptor) :
	caster(caster),
	descriptor(descriptor)
{}

Spell::~Spell() {}

bool Spell::isActive() const {
	return active;
}

SpellDesc* Spell::getDescriptor() const {
	return descriptor;
}

shared_ptr<SpellDesc> Spell::getDescriptor(const string& name)
{
	auto it = spellDescriptors.find(name);

	if (it != spellDescriptors.end())
		return it->second;
	else
		return nullptr;
}

const string Teleport::name = "Teleport";
const string Teleport::description = "";

SpellGeneratorType Teleport::make_generator(const vector<object_ref<TeleportPad>>& targets)
{
	return [targets](GObject* caster) -> shared_ptr<Spell> {
		return make_shared<Teleport>(caster, targets);
	};
}

Teleport::Teleport(GObject* caster, const vector<object_ref<TeleportPad>>& targets) :
	Spell(caster, {}, Spell::getDescriptor("Teleport").get()),
	targets(targets)
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

StarlightTyphoon::StarlightTyphoon(GObject* caster, const ValueMap& args, SpellDesc* descriptor):
Spell(caster, args,descriptor)
{
    set_float_arg(count, 30.0f)
    set_float_arg(duration, 1.0f)
    set_float_arg(speed, 6.0f)
    set_float_arg(width, float_pi / 4.0f)
    set_float_arg(radius, 0.2f)
    
	angle = getFloatOrDefault(args, "angle", caster->getAngle());

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
		arcPos,
		pos,
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

IllusionDial::IllusionDial(GObject* caster,const ValueMap& args, SpellDesc* descriptor) :
Spell(caster,args,descriptor),
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
			caster->getPos() + SpaceVect::ray(radius, arc_start + i * arc_spacing),
			i % 2 ? angular_speed : -angular_speed
		));
    }
}

void IllusionDial::runPeriodic()
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
    //delete remaining bullets
}
