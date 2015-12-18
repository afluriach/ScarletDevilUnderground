//
//  Spell.hpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

class GObject;
class Spellcaster;

#define STANDARD_CONS(name) inline name(Spellcaster* caster,const ValueMap& args) : Spell(caster,args) {}

class Spell
{
public:
    typedef function<shared_ptr<Spell>(Spellcaster*,const ValueMap&)> AdapterType;
    static const unordered_map<string,AdapterType> adapters;
    static const set<string> scripts;

    inline Spell(Spellcaster* caster,const ValueMap& args) : caster(caster){
    }
    
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void end() = 0;
protected:
    Spellcaster* caster;
};

class FlameFence : public Spell{
public:
    STANDARD_CONS(FlameFence)
    void init();
    void update();
    void end();
protected:
    vector<GObject*> bullets;
};

//A somewhat conical, but mostly focused attack.
//Most of the shots to be within half of the cone width.
//Speed variation: maybe a factor of two.
//Size variation, similarly, and the two should be inversely correlated.
class StarlightTyphoon : public Spell{
public:
    inline StarlightTyphoon(Spellcaster* caster, const ValueMap& args):
    Spell(caster, args)
    {
        set_float_arg(count, 30)
        set_float_arg(duration, 1)
        set_float_arg(speed, 6)
        set_float_arg(width, float_pi / 4)
        set_float_arg(angle, 0)
        set_float_arg(radius, 0.2)
        
        shotsPerFrame = count / duration * App::secondsPerFrame;
    }
    void init();
    void update();
    void end();
    void fire();
protected:
    //Distance from caster
    const float offset = 0.7;
    
    float elapsed = 0;
    float shotsPerFrame;
    float accumulator = 0;

    //Projectiles will be uniformly generated over the time period.
    float count;
    float duration;

    //Average speed.
    float speed;
    //Width of the cone.
    float width;
    //Direction
    float angle;
    //Average bullet radius
    float radius;
};

class PeriodicSpell : virtual public Spell{
public:
    virtual float interval() const = 0;
    virtual void runPeriodic() = 0;
    inline void update(){
        timeSince += App::secondsPerFrame;
        
        if(timeSince >= interval()){
            timeSince -= interval();
            runPeriodic();
        }
    }
protected:
    float timeSince = 0;
};

class FireStarburst : public PeriodicSpell{
public:
    static constexpr float bulletSpeed = 6;
    STANDARD_CONS(FireStarburst)
    no_op(init);
    inline float interval() const {return 0.5;}
    void runPeriodic();
    no_op(end);
};

class ScriptedSpell : public Spell{
public:
    ScriptedSpell(Spellcaster* caster, const string& scriptRes, const ValueMap& args);
    virtual void init();
    virtual void update();
    virtual void end();

protected:
    Lua::Inst ctx;
    string name;
    //Would save the constructor argument for sending to init.
    //LuaRef luaArgs;
};

#endif /* Spell_hpp */
