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

#define STANDARD_CONS(name) inline name(GObject* caster) : Spell(caster) {}
#define GET_DESC(name) virtual inline shared_ptr<SpellDesc> getDescriptor() { return Spell::getDescriptorByName(#name); }

typedef function<shared_ptr<Spell>(GObject*)> SpellGeneratorType;

class SpellDesc;
class TeleportPad;
class Torch;

class Spell
{
public:
    static const unordered_map<string,shared_ptr<SpellDesc>> spellDescriptors;

	static shared_ptr<SpellDesc> getDescriptorByName(const string& name);

	Spell(GObject* caster);
	virtual ~Spell();
    
	bool isActive() const;

	template<class T>
	inline T* getCasterAs(){
		return dynamic_cast<T*>(caster);
	}

	virtual shared_ptr<SpellDesc> getDescriptor() = 0;

    virtual void init() = 0;
    virtual void update() = 0;
    virtual void end() = 0;
protected:
    GObject* caster;
    bool active = true;
};

template<class C>
inline static SpellGeneratorType make_spell_generator()
{
	return [](GObject* caster) -> shared_ptr<Spell> {
		return make_shared<C>(caster);
	};
}

class Teleport : public Spell {
public:
	static const string name;
	static const string description;

	Teleport(GObject* caster);
	inline virtual ~Teleport() {}

	GET_DESC(Teleport)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	vector<object_ref<TeleportPad>> targets;
	object_ref<TeleportPad> toUse;
};

//A somewhat conical, but mostly focused attack.
//Most of the shots to be within half of the cone width.
//Speed variation: maybe a factor of two.
//Size variation, similarly, and the two should be inversely correlated.
class StarlightTyphoon : public Spell{
public:
	static const string name;
	static const string description;

	static const SpaceFloat speed;
	static const SpaceFloat width;
	static const SpaceFloat radius;
	static const SpaceFloat duration;
	static const SpaceFloat offset;

	static const unsigned int count;

    StarlightTyphoon(GObject* caster);
	inline virtual ~StarlightTyphoon() {}

	GET_DESC(StarlightTyphoon)
    virtual void init();
    virtual void update();
    virtual void end();

    void fire();
protected:
    SpaceFloat elapsed = 0.0;
    SpaceFloat shotsPerFrame;
    SpaceFloat accumulator = 0.0;
	SpaceFloat angle;
};

class PeriodicSpell : public Spell{
public:

	inline PeriodicSpell(GObject* caster) : Spell(caster) {}
	inline virtual ~PeriodicSpell() {}

    virtual float interval() const = 0;
    virtual void runPeriodic() = 0;
	void update();
protected:
    float timeSince = 0;
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

class IllusionDial : public Spell
{
public:
	static const string name;
	static const string description;

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

	IllusionDial(GObject* caster);
	inline virtual ~IllusionDial() {}
    
	virtual void update();

	GET_DESC(IllusionDial)
    virtual void init();
    virtual void end();
};

class TorchDarkness : public Spell
{
public:
	static const SpaceFloat radius;
	static const float effectTime;

	TorchDarkness(GObject* caster);
	inline virtual ~TorchDarkness() {}

	GET_DESC(TorchDarkness)
	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	map<Torch*, float> torches;
};

class NightSignPinwheel : public Spell
{
public:
	static const string name;
	static const string description;

	static const int legCount;
	static const int bulletsPerLegCount;
	static const SpaceFloat launchDelay;
	static const SpaceFloat refreshRatio;
	static const SpaceFloat legLength;
	static const SpaceFloat legStartDist;
	static const SpaceFloat legAngleSkew;

	NightSignPinwheel(GObject* caster);
	inline virtual ~NightSignPinwheel() {}

	GET_DESC(NightSignPinwheel)
	inline virtual void init() {}
	virtual void update();
	virtual void end();
protected:
	SpaceFloat bulletsExistingRatio();
	void generate();
	void generateLeg(SpaceFloat angle);
	void launch();
	void removeBullets();

	unordered_set<gobject_ref> bullets;
	SpaceFloat timer = 0.0;
	bool waitingToLaunch = false;
};

#endif /* Spell_hpp */
