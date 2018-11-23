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
#include "value_map.hpp"

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

ScriptedSpell::ScriptedSpell(GObject* caster, const string& scriptRes, const ValueMap& args):
Spell(caster, args),
//luaArgs(Lua::convert<ValueMap>::convertToLua(args, ctx.state)),
ctx(boost::lexical_cast<string>(caster->uuid) + "_" + scriptRes)
{
    ctx.runFile("scripts/spells/" + scriptRes + ".lua");
    //Push caster and args as global variables in the script's context.
    ctx.setGlobal<GObject*>(caster, "caster");
    ctx.setGlobal(args, "args");
}

void ScriptedSpell::init(){
    //ctx.callIfExistsNoReturn("init", {luaArgs});
    ctx.callIfExistsNoReturn("init");
}
void ScriptedSpell::update(){
    ctx.callIfExistsNoReturn("update");
}
void ScriptedSpell::end(){
    ctx.callIfExistsNoReturn("exit");
}

StarlightTyphoon::StarlightTyphoon(GObject* caster, const ValueMap& args):
Spell(caster, args)
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

const int IllusionDial::count = 16;

const float IllusionDial::radius = 2.5f;
const float IllusionDial::arc_start = 0.0f;
const float IllusionDial::arc_end = float_pi * 2.0f;
const float IllusionDial::arc_width = arc_end - arc_start;
const float IllusionDial::arc_spacing = arc_width / (count-1);
const float IllusionDial::angular_speed = float_pi * 2.0f / 3.0f;

const float IllusionDial::max_angle_margin = float_pi / 12.0f;
const float IllusionDial::min_fire_interval = 1.0f / 3.0f;

IllusionDial::IllusionDial(GObject* caster,const ValueMap& args) :
Spell(caster,args),
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

const int PlayerBatMode::framesPerDrain = App::framesPerSecond / Player::batModeCostPerSecond;

PlayerBatMode::PlayerBatMode(Player* caster,const ValueMap& args) :
Spell(caster,args)
{
    p = caster;
}

void PlayerBatMode::init()
{
    p->consumePower(Player::batModeInitialCost);

    p->setSpellProtectionMode(true);
    p->setSprite("flandre_bat");
    p->setMaxSpeed(Player::batModeMaxSpeed);
}
void PlayerBatMode::update()
{
    if(framesSinceDrain >= framesPerDrain){
        if(!p->consumePower(1)){
            active = false;
        }
        framesSinceDrain = 0;
    }
    p->checkBatModeControls();
    ++framesSinceDrain;
}

void PlayerBatMode::end()
{
    p->setSpellProtectionMode(false);
    p->setSprite("flandre");
    p->setMaxSpeed(Player::baseMaxSpeed);
	p->onSpellStop();
}
