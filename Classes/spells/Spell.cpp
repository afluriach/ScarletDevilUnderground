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
#include "Player.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "TeleportPad.hpp"
#include "util.h"
#include "value_map.hpp"

shared_ptr<SpellDesc> Spell::getDescriptor(const string& name)
{
	auto it = spellDescriptors.find(name);

	if (it != spellDescriptors.end())
		return it->second;
	else
		return nullptr;
}

const string FireStarburst::name = "FireStarburst";
const string FireStarburst::description = "";

const int FireStarburst::initialCost = 0;
const int FireStarburst::costPerSecond = 0;

void FireStarburst::runPeriodic()
{
    SpaceVect pos = caster->body->getPos();
    for_irange(i,0,8)
    {
        SpaceFloat angle = float_pi * i / 4.0;

        SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

		caster->space->createObject(
			GObject::make_object_factory<FireBullet>(angle, pos, bulletSpeed)
		);
    }
}

const string FlameFence::name = "FlameFence";
const string FlameFence::description = "";

const int FlameFence::initialCost = 0;
const int FlameFence::costPerSecond = 0;

FlameFence::FlameFence(GObject* caster) :
Spell(caster, {}, Spell::getDescriptor("FlameFence").get())
{
}

void FlameFence::init()
{
    SpaceVect center = caster->body->getPos();
    
    for(int y = -10; y < 10; y += 2)
    {
        SpaceVect rowSkew(y % 2 ? 0.5 : 0, 0);
        for(int x = -10; x < 10; x += 2)
        {
            SpaceVect pos(center);
            pos += SpaceVect(x,y) + rowSkew;
            
			bullets.push_back(caster->space->createObject(
				GObject::make_object_factory<FireBullet>(0.0f, pos, 0.0f)
			));
        }
    }
}

void FlameFence::update()
{
}

void FlameFence::end()
{
    for(gobject_ref bullet: bullets){
        if(bullet.isValid())
            caster->space->removeObject(bullet.get());
    }
}

const string Whirlpool1::name = "Whirlpool1";
const string Whirlpool1::description = "";

const int Whirlpool1::initialCost = 0;
const int Whirlpool1::costPerSecond = 0;

const int Whirlpool1::shotsPerSecond = 4;

const SpaceFloat Whirlpool1::angularSpeed = float_pi / 6.0;
const SpaceFloat Whirlpool1::angularOffset = float_pi / 12.0;
const SpaceFloat Whirlpool1::bulletSpeed = 6.0;

Whirlpool1::Whirlpool1(GObject* caster) :
	Spell(caster, {}, Spell::getDescriptor("Whirlpool1").get())
{
}

void Whirlpool1::init()
{
	caster->setAngularVel(angularSpeed);
}

void Whirlpool1::update()
{
	timerDecrement(shotTimer);

	SpaceFloat angle = caster->getAngle();
	SpaceVect pos = caster->getPos();

	array<SpaceFloat, 6> angles;

	angles[0] = angle - angularOffset;
	angles[1] = angle;
	angles[2] = angle + angularOffset;

	angles[3] = float_pi + angle - angularOffset;
	angles[4] = float_pi + angle;
	angles[5] = float_pi + angle + angularOffset;

	if (shotTimer <= 0.0) {

		for_irange(i, 0, 6) {
			caster->space->createObject(GObject::make_object_factory<WaterBullet>(
				angles[i],
				pos + SpaceVect::ray(1.0, angles[i]),
				bulletSpeed
			));
		}

		shotTimer = 1.0 / shotsPerSecond;
	}
}

void Whirlpool1::end()
{

}

const string Whirlpool2::name = "Whirlpool2";
const string Whirlpool2::description = "";

const int Whirlpool2::initialCost = 0;
const int Whirlpool2::costPerSecond = 0;

const int Whirlpool2::shotsPerSecond = 6;

const SpaceFloat Whirlpool2::angularSpeed = float_pi / 5.0;
const SpaceFloat Whirlpool2::angularOffset = float_pi / 10.0;
const SpaceFloat Whirlpool2::bulletSpeed = 7.5;

Whirlpool2::Whirlpool2(GObject* caster) :
Spell(caster, {}, Spell::getDescriptor("Whirlpool2").get())
{
}

void Whirlpool2::init()
{
	caster->setAngularVel(angularSpeed);
}

void Whirlpool2::update()
{
	timerDecrement(shotTimer);

	SpaceFloat angle = caster->getAngle();
	SpaceVect pos = caster->getPos();

	array<SpaceFloat, 12> angles;

	angles[0] = angle - angularOffset;
	angles[1] = angle;
	angles[2] = angle + angularOffset;

	angles[3] = (float_pi / 2.0) + angle - angularOffset;
	angles[4] = (float_pi / 2.0) + angle;
	angles[5] = (float_pi / 2.0) + angle + angularOffset;

	angles[6] = float_pi + angle - angularOffset;
	angles[7] = float_pi + angle;
	angles[8] = float_pi + angle + angularOffset;

	angles[9] = (float_pi * 1.5) + angle - angularOffset;
	angles[10] = (float_pi * 1.5) + angle;
	angles[11] = (float_pi * 1.5) + angle + angularOffset;


	if (shotTimer <= 0.0) {

		for_irange(i, 0, 12) {
			caster->space->createObject(GObject::make_object_factory<WaterBullet>(
				angles[i],
				pos + SpaceVect::ray(1.0, angles[i]),
				bulletSpeed
				));
		}

		shotTimer = 1.0 / shotsPerSecond;
	}
}

void Whirlpool2::end()
{

}

const string Teleport::name = "Teleport";
const string Teleport::description = "";

const int Teleport::initialCost = 0;
const int Teleport::costPerSecond = 0;

SpellGeneratorType Teleport::make_generator(const vector<object_ref<TeleportPad>>& targets)
{
	return [targets](GObject* caster) -> unique_ptr<Spell> {
		return make_unique<Teleport>(caster, targets);
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

const int StarlightTyphoon::initialCost = 0;
const int StarlightTyphoon::costPerSecond = 0;

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

const int IllusionDial::initialCost = 0;
const int IllusionDial::costPerSecond = 0;

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

void PlayerSpell::init()
{
	Player* p = getCasterAs<Player>();

	p->consumePower(getDescriptor()->getInitialCost());
}

void PlayerSpell::update()
{
	Player* p = getCasterAs<Player>();

	powerDrainAccumulator += App::secondsPerFrame * getDescriptor()->getCostPerSecond();

	while (powerDrainAccumulator >= 1.0f)
	{
		if (!p->consumePower(1)) {
			active = false;
		}
		powerDrainAccumulator -= 1.0f;
	}
}

const string PlayerBatMode::name = "PlayerBatMode";
const string PlayerBatMode::description = "";

const int PlayerBatMode::initialCost = 9;
const int PlayerBatMode::costPerSecond = 5;

PlayerBatMode::PlayerBatMode(GObject* caster,const ValueMap& args, SpellDesc* descriptor) :
Spell(caster,args,descriptor)
{}

void PlayerBatMode::init()
{
	PlayerSpell::init();

	Player* p = getCasterAs<Player>();

	if (p) {
		p->setProtection();
		p->setFiringSuppressed(true);
		p->setSprite("flandre_bat");
		p->applyAttributeModifier(Attribute::speed, 1.5f);
		p->setLayers(PhysicsLayers::ground);
	}
}

void PlayerBatMode::end()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->resetProtection();
		p->setFiringSuppressed(false);
		p->setSprite("flandre");
		p->applyAttributeModifier(Attribute::speed, -1.5f);
		p->setLayers(enum_bitwise_or(PhysicsLayers, floor, ground));
		p->onSpellStop();
	}
}

const string PlayerDarkMist::name = "PlayerDarkMist";
const string PlayerDarkMist::description = "";

const int PlayerDarkMist::initialCost = 10;
const int PlayerDarkMist::costPerSecond = 5;

PlayerDarkMist::PlayerDarkMist(GObject* caster, const ValueMap& args, SpellDesc* descriptor) :
	Spell(caster, args, descriptor)
{}

void PlayerDarkMist::init()
{
	PlayerSpell::init();

	caster->sprite->setOpacity(128);
	caster->setInvisible(true);
}

void PlayerDarkMist::end()
{
	caster->sprite->setOpacity(255);
	caster->setInvisible(false);
}

const string PlayerIceShield::name = "PlayerIceShield";
const string PlayerIceShield::description = "";

const int PlayerIceShield::initialCost = 1;
const int PlayerIceShield::costPerSecond = 0;

const SpaceFloat PlayerIceShield::speed = 9.0;

const SpaceFloat PlayerIceShield::distance = 3.0;
const SpaceFloat PlayerIceShield::circumference = 2.0 * float_pi * distance;
const SpaceFloat PlayerIceShield::inv_circumference = 1.0 / circumference;

PlayerIceShield::PlayerIceShield(GObject* caster, const ValueMap& args, SpellDesc* descriptor) :
	Spell(caster, args, descriptor)
{}

void PlayerIceShield::init()
{
	PlayerSpell::init();

	SpaceVect origin = caster->getPos();

	for_irange(i,0,bulletCount)
	{
		SpaceFloat angle = (1.0 * i / bulletCount) * (float_pi * 2.0);
		SpaceVect pos = SpaceVect::ray(distance, angle);
		
		bullets[i] = caster->space->createObject(
			GObject::make_object_factory<CirnoIceShieldBullet>(
				angle - float_pi / 2.0,
				origin + pos
			)
		);
	}
}

void PlayerIceShield::update()
{
	PlayerSpell::update();

	//Update angle based on linear speed
	SpaceFloat dp = App::secondsPerFrame * speed;
	crntAngle += dp * inv_circumference * float_pi * 2.0;

	if (crntAngle >= 2.0*float_pi) {
		crntAngle -= 2.0*float_pi;
	}

	SpaceVect origin = caster->getPos();

	for_irange(i, 0, bulletCount)
	{
		SpaceFloat angle = (1.0 * i / bulletCount) * (float_pi * 2.0) + crntAngle;
		SpaceVect pos = SpaceVect::ray(distance, angle);
		SpaceVect vel = SpaceVect::ray(speed, angle - float_pi/2.0); 

		if (bullets[i].isValid()) {
			bullets[i].get()->setPos(origin + pos);
			bullets[i].get()->setVel(vel);
			bullets[i].get()->setAngle(angle);
		}
	}
}

void PlayerIceShield::end()
{
	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			caster->space->removeObject(ref.get());
		}
	}
}

