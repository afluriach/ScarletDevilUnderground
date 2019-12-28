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

	enum class state
	{
		created = 1,
		active,
		ending,
		expired,
	};

    static const unordered_map<string,shared_ptr<SpellDesc>> spellDescriptors;
	static const vector<string> playerSpells;
	static const vector<string> playerPowerAttacks;

	static shared_ptr<SpellDesc> getDescriptorByName(const string& name);

	//length: -1 means indefinite, 0 means immediate
	//updateInterval: -1 means no update, 0 means every frame, units in seconds.
	Spell(GObject* caster, spell_params params);
	virtual ~Spell();
    
	bool isActive() const;

	template<class T>
	inline T* getCasterAs(){
		return dynamic_cast<T*>(caster);
	}

	GSpace* getSpace() const;

	virtual shared_ptr<SpellDesc> getDescriptor() = 0;

	inline virtual void init() {}
	inline virtual void update() {}
	inline virtual void end() {}
protected:
	void runUpdate();
	void runEnd();

	//spell_params
	spell_cost _cost;
	SpaceFloat length;
	SpaceFloat updateInterval;

	SpaceFloat t = 0.0;
	SpaceFloat lastUpdate = 0.0;

    GObject* caster;
	state crntState = state::created;
};

template<class C>
inline SpellGeneratorType make_spell_generator()
{
	return [](GObject* caster) -> shared_ptr<Spell> {
		return make_shared<C>(caster);
	};
}

class ScriptedSpell : public Spell {
public:
	static spell_params getParams(string clsName);
	static SpellGeneratorType generator(string clsName);

	ScriptedSpell(GObject* caster, string clsName);
	virtual ~ScriptedSpell();

	virtual shared_ptr<SpellDesc> getDescriptor();
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
	ApplySelfEffect(GObject* caster, spell_params params, shared_ptr<MagicEffectDescriptor> effect);

	virtual void init();
	virtual void end();
protected:
	shared_ptr<MagicEffectDescriptor> effect;
	unsigned int effectID = 0;
};

#endif /* Spell_hpp */
