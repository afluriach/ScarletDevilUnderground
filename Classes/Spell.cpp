//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"
#include "Bullet.hpp"
#include "Spell.hpp"

void FireStarburst::runPeriodic()
{
    SpaceVect pos = caster->body->getPos();
    for(int i=0;i<8; ++i)
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
