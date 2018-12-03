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
#include "value_map.hpp"

const string FireStarburst::name = "FireStarburst";
const string FireStarburst::description = "";

const int FireStarburst::initialCost = 0;
const int FireStarburst::costPerSecond = 0;

void FireStarburst::runPeriodic()
{
    SpaceVect pos = caster->body->getPos();
    for_irange(i,0,8)
    {
        float angle = float_pi * i / 4;

        SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

        FireBullet* bullet = new FireBullet(crntPos);
        bullet->setInitialVelocity(SpaceVect::ray(bulletSpeed, angle));
        app->space->addObject(bullet);
    }
}

const string FlameFence::name = "FlameFence";
const string FlameFence::description = "";

const int FlameFence::initialCost = 0;
const int FlameFence::costPerSecond = 0;

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
            
            FireBullet* bullet = new FireBullet(pos);
            bullets.push_back(bullet);
            app->space->addObject(bullet);
        }
    }
}

void FlameFence::update()
{
}

void FlameFence::end()
{
    foreach(gobject_ref bullet, bullets){
        if(bullet.isValid())
            app->space->removeObject(bullet.get());
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
    set_float_arg(angle, 0.0f)
    set_float_arg(radius, 0.2f)
    
    shotsPerFrame = count / duration * App::secondsPerFrame;
}


void StarlightTyphoon::init()
{
}
void StarlightTyphoon::fire()
{
    //The angle variation, will be added to the base direction.
    float arcPos = app->getRandomFloat(-width, width);
    float crntSpeed = app->getRandomFloat(speed*0.5, speed*1.5);
    float crntRadius = app->getRandomFloat(radius*0.7, radius*1.3);
    
    SpaceVect pos = caster->getPos() + SpaceVect::ray(offset, angle);
    
    StarBullet* b = new StarBullet(
        pos,
        crntRadius,
        StarBullet::colors[app->getRandomInt(0,StarBullet::colors.size()-1)]
    );
    b->setInitialVelocity(SpaceVect::ray(crntSpeed, arcPos + angle));
    app->space->addObject(b);
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
  //      spawn_points[i] = SpaceVect::ray(radius, arc_start + i*arc_spacing);
        IllusionDialDagger* bullet = new IllusionDialDagger(
            caster->getPos() + SpaceVect::ray(radius, arc_start + i*arc_spacing),
            i % 2 ? angular_speed : -angular_speed
        );
        app->space->addObject(bullet);
        bullets[i] = object_ref<IllusionDialDagger>(bullet);
    }
}

void IllusionDial::runPeriodic()
{
	++framesSinceLastFire;

	if (framesSinceLastFire*App::secondsPerFrame >= min_fire_interval)
	{

		int best = -1;
		float best_angle = float_pi;
		bool allBulletsConsumed = true;

		for_irange(i, 0, count)
		{
			if (bullets[i].isValid() && !launch_flags[i]) {
				allBulletsConsumed = false;

				float crnt = bullets[i].get()->targetViewAngle();

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

    p->setSpellProtectionMode(true);
	p->setFiringSuppressed(true);
    p->setSprite("flandre_bat");
	p->applyAttributeModifier(Attribute::speed, 1.5f);
	p->setLayers(PhysicsLayers::ground);
}

void PlayerBatMode::end()
{
	Player* p = getCasterAs<Player>();

    p->setSpellProtectionMode(false);
	p->setFiringSuppressed(false);
    p->setSprite("flandre");
	p->applyAttributeModifier(Attribute::speed, -1.5f);
	p->setLayers(enum_bitwise_or(PhysicsLayers, floor, ground));
	p->onSpellStop();
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
