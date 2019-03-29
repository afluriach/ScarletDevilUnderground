//
//  Spell.hpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

#include "types.h"

#define STANDARD_CONS(name) inline name(GObject* caster) : Spell(caster) {}
#define GET_DESC(name) virtual inline shared_ptr<SpellDesc> getDescriptor() { return Spell::getDescriptorByName(#name); }

class GObject;
class SpellDesc;
class TeleportPad;
class Torch;

class Spell
{
public:
    static const unordered_map<string,shared_ptr<SpellDesc>> spellDescriptors;
	static const vector<string> playerSpells;
	static const vector<string> playerPowerAttacks;

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

#endif /* Spell_hpp */
