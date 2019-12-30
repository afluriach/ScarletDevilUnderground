//
//  SpellSystem.hpp
//  Koumachika
//
//  Created by Toni on 12/30/19.
//
//

#ifndef SpellSystem_hpp
#define SpellSystem_hpp

class Spell;

class SpellSystem
{
public:
	friend class GSpace;

	SpellSystem(GSpace* gspace);

	unsigned int cast(shared_ptr<SpellDesc> desc, GObject* caster);

	void stopSpell(unsigned int id);
	bool isSpellActive(unsigned int id);
protected:
	void applyRemove(unsigned int id);
	void stopObjectSpells(GObject* obj);
	void update();
	
	list<unsigned int> toRemove;
	map<unsigned int, Spell*> spells;
	multimap<GObject*, unsigned int> objectSpells;

	GSpace* gspace;
	unsigned int nextID = 1;
};

#endif
