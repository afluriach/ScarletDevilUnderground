//
//  Spell.cpp
//  FlansBasement
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
        float angle = boost::math::float_constants::pi * i / 4;

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

ScriptedSpell::ScriptedSpell(GObject* caster, const string& scriptRes):
Spell(caster),
ctx(boost::lexical_cast<string>(caster->uuid) + "_" + scriptRes)
{
    ctx.runFile("scripts/spells/" + scriptRes + ".lua");
    //Push caster as a global variable in the script's context.
    ctx.setGlobal(Lua::convert<GObject*>::convertToLua(caster, ctx.state), "caster");
}

void ScriptedSpell::init(){
    ctx.callIfExistsNoReturn("init");
}
void ScriptedSpell::update(){
    ctx.callIfExistsNoReturn("update");
}
void ScriptedSpell::end(){
    ctx.callIfExistsNoReturn("exit");
}

