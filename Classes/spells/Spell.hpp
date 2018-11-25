//
//  Spell.hpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

#include "App.h"
#include "GObject.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "object_ref.hpp"
#include "types.h"
#include "util.h"

#define STANDARD_CONS(name) inline name(GObject* caster,const ValueMap& args, SpellDesc* descriptor) : Spell(caster,args, descriptor) {}

class SpellDesc;

class Spell
{
public:
    static const unordered_map<string,SpellDesc*> spellDescriptors;

    inline Spell(GObject* caster,const ValueMap& args, SpellDesc* descriptor) :
		caster(caster),
		descriptor(descriptor)
	{}
    
	inline virtual ~Spell() {}
    
    inline bool isActive() const{
        return active;
    }

	template<class T>
	inline T* getCasterAs(){
		return dynamic_cast<T*>(caster);
	}

	inline SpellDesc* getDescriptor() const {
		return descriptor;
	}

    virtual void init() = 0;
    virtual void update() = 0;
    virtual void end() = 0;
protected:
	SpellDesc * descriptor;
    GObject* caster;
    bool active = true;
};

class FlameFence : public Spell{
public:
	static const string name;
	static const string description;

	static const int initialCost;
	static const int costPerSecond;

    STANDARD_CONS(FlameFence)
    void init();
    void update();
    void end();
protected:
    vector<gobject_ref> bullets;
};

//A somewhat conical, but mostly focused attack.
//Most of the shots to be within half of the cone width.
//Speed variation: maybe a factor of two.
//Size variation, similarly, and the two should be inversely correlated.
class StarlightTyphoon : public Spell{
public:
	static const string name;
	static const string description;

	static const int initialCost;
	static const int costPerSecond;

    StarlightTyphoon(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
    void init();
    void update();
    void end();
    void fire();
protected:
    //Distance from caster
    const float offset = 0.7f;
    
    float elapsed = 0.0f;
    float shotsPerFrame;
    float accumulator = 0.0f;

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

	inline PeriodicSpell() {}
	inline virtual ~PeriodicSpell() {}

    virtual float interval() const = 0;
    virtual void runPeriodic() = 0;
    inline void update(){

		float _interval = interval();

		if (_interval == 0.0f)
		{
			runPeriodic();
		}

		else
		{
			timeSince += App::secondsPerFrame;

			if (timeSince >= _interval) {
				timeSince -= _interval;
				runPeriodic();
			}
		}
    }
protected:
    float timeSince = 0;
};

class FireStarburst : public PeriodicSpell{
public:
	static const string name;
	static const string description;

	static const int initialCost;
	static const int costPerSecond;

    static constexpr float bulletSpeed = 6.0f;

    STANDARD_CONS(FireStarburst)
    no_op(init);
    inline float interval() const {return 0.5f;}
    void runPeriodic();
    no_op(end);
};

//class PuppetryDoll : public Spell
//{
//
//    inline PuppetryDoll(Spellcaster* caster,const ValueMap& args) :
//    Spell(caster,args)
//    {}
//    
//    virtual void init();
//    virtual void update();
//    virtual void end();
//
//};

class IllusionDialDagger;

class IllusionDial : public PeriodicSpell
{
public:
	static const string name;
	static const string description;

	static const int initialCost;
	static const int costPerSecond;

    static const float radius;
    static const float arc_start;
    static const float arc_end;
    static const float arc_width;
    static const float arc_spacing;
    static const float angular_speed;

	static const float max_angle_margin;
	static const float min_fire_interval;

    static const int count;
    
    vector<object_ref<IllusionDialDagger>> bullets;
    vector<bool> launch_flags;
	unsigned int framesSinceLastFire = 0;

	IllusionDial(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
    
    inline virtual float interval() const {return 0.0f;};
    
    virtual void init();
    virtual void runPeriodic();
    virtual void end();
};

//Apply power drain and deactive spell if applicable
class PlayerSpell : virtual public Spell {
public:
	inline PlayerSpell() {}
	inline virtual ~PlayerSpell() {}

	virtual void init();
	virtual void update();
protected:
	float powerDrainAccumulator = 0.0f;
};

class PlayerBatMode : virtual public Spell, public PlayerSpell{
public:
	static const string name;
	static const string description;

	static const int initialCost;
	static const int costPerSecond;

	PlayerBatMode(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
    virtual void init();
    virtual void end();
protected:
    int framesSinceDrain = 0;
};

class PlayerDarkMist : virtual public Spell, public PlayerSpell {
public:
	static const string name;
	static const string description;

	static const int initialCost;
	static const int costPerSecond;

	PlayerDarkMist(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
	virtual void init();
	virtual void end();
protected:
	int framesSinceDrain = 0;
};


#endif /* Spell_hpp */