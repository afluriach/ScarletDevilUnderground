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
#include "scenes.h"
#include "util.h"


void FireStarburst::runPeriodic()
{
    SpaceVect pos = caster->body->getPos();
    for_irange(i,0,8)
    {
        float angle = float_pi * i / 4;

        SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

        FireBullet* bullet = new FireBullet(crntPos);
        bullet->setInitialVelocity(SpaceVect::ray(bulletSpeed, angle));
        GScene::getSpace()->addObject(bullet);
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
            GScene::getSpace()->addObject(bullet);
        }
    }
}

void FlameFence::update()
{
}

void FlameFence::end()
{
    foreach(GObject* bullet, bullets){
        GScene::getSpace()->removeObject(bullet);
    }
}

ScriptedSpell::ScriptedSpell(Spellcaster* caster, const string& scriptRes, const ValueMap& args):
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
    GScene::getSpace()->addObject(b);
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
        caster->stop();
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

IllusionDial::IllusionDial(Spellcaster* caster,const ValueMap& args) :
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
        GScene::getSpace()->addObject(bullet);
        bullets[i] = object_ref(bullet);
    }
}

void IllusionDial::runPeriodic()
{
    int best = -1;
    float best_angle = float_pi;
    
    for_irange(i,0,count)
    {
        GObject* obj = bullets[i].get();
        IllusionDialDagger* bullet = nullptr;
        
        if(obj && !launch_flags[i]){
            bullet = dynamic_cast<IllusionDialDagger*>(obj);
        }
        
        if(bullet){
            float crnt = bullet->targetViewAngle();
            
            if(!isinf(crnt) && abs(crnt) < best_angle)
                best = i;
                best_angle = abs(crnt);
        }
    }
    
    if(best != -1)
    {
        dynamic_cast<IllusionDialDagger*>(bullets[best].get())->launch();
        launch_flags[best] = true;
    }
    else
    {
        //Deactivate spell if all bullets are consumed.
        active = false;
    }
}

void IllusionDial::end()
{
    //delete remaining bullets
}
