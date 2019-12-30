//
//  Spell.hpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

#include "spell_types.hpp"

#define STANDARD_CONS(name) inline name(GObject* caster) : Spell(caster) {}
#define GET_DESC(name) virtual inline shared_ptr<SpellDesc> getDescriptor() { return Spell::getDescriptorByName(#name); }

class GObject;
class MagicEffectDescriptor;
class SpellDesc;
class TeleportPad;
class Torch;

class Spell
{
public:
	friend class GObject;
	friend class SpellSystem;

    static const unordered_map<string,shared_ptr<SpellDesc>> spellDescriptors;
	static const vector<string> playerSpells;
	static const vector<string> playerPowerAttacks;

	static shared_ptr<SpellDesc> getDescriptorByName(const string& name);

	//length: -1 means indefinite, 0 means immediate
	//updateInterval: -1 means no update, 0 means every frame, units in seconds.
	Spell(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id, spell_params params);
	virtual ~Spell();
    
	template<class T>
	inline T* getCasterAs() const{
		return dynamic_cast<T*>(caster);
	}

	GSpace* getSpace() const;

	bullet_attributes getBulletAttributes(shared_ptr<bullet_properties> props) const;

	gobject_ref spawnBullet(
		shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceVect velocity,
		SpaceFloat angle,
		SpaceFloat angularVelocity
	);
	gobject_ref launchBullet(
		shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceFloat angle,
		SpaceFloat angularVelocity = 0.0,
		bool obstacleCheck = true
	);

	inline shared_ptr<SpellDesc> getDescriptor() const { return descriptor; }

	inline virtual void init() {}
	inline virtual void update() {}
	inline virtual void end() {}

	void stop();
protected:
	void runUpdate();

	//spell_params
	spell_cost _cost;
	SpaceFloat length;
	SpaceFloat updateInterval;

	SpaceFloat t = 0.0;
	SpaceFloat lastUpdate = 0.0;

	shared_ptr<SpellDesc> descriptor;
    GObject* caster;
	unsigned int id;
};

class ScriptedSpell : public Spell {
public:
	static spell_params getParams(string clsName);

	ScriptedSpell(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id, string clsName);
	virtual ~ScriptedSpell();

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	string clsName;
	sol::table obj;
};

class ApplySelfEffect : public Spell
{
public:
	ApplySelfEffect(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id, spell_params params, shared_ptr<MagicEffectDescriptor> effect);

	virtual void init();
	virtual void end();
protected:
	shared_ptr<MagicEffectDescriptor> effect;
	unsigned int effectID = 0;
};

#endif /* Spell_hpp */
