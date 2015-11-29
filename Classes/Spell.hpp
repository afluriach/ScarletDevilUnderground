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

#define STANDARD_CONS(name) inline name(GObject* caster) : Spell(caster) {}

class Spell
{
public:
    typedef function<shared_ptr<Spell>(GObject*)> AdapterType;
    static const map<string,AdapterType> adapters;

    inline Spell(GObject* caster) : caster(caster){
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
    inline FireStarburst(GObject* caster) : Spell(caster) {}
    no_op(init);
    inline float interval() const {return 0.5;}
    void runPeriodic();
    no_op(end);
};

#endif /* Spell_hpp */