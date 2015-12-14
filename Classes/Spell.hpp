//
//  Spell.hpp
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

class GObject;

#define STANDARD_CONS(name) inline name(GObject* caster,const ValueMap& args) : Spell(caster,args) {}

class Spell
{
public:
    typedef function<shared_ptr<Spell>(GObject*,const ValueMap&)> AdapterType;
    static const unordered_map<string,AdapterType> adapters;
    static const set<string> scripts;

    inline Spell(GObject* caster,const ValueMap& args) : caster(caster){
    }
    
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void end() = 0;
protected:
    GObject* caster;
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
    ScriptedSpell(GObject* caster, const string& scriptRes, const ValueMap& args);
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
