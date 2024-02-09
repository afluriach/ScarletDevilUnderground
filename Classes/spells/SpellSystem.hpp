//
//  SpellSystem.hpp
//  Koumachika
//
//  Created by Toni on 12/30/19.
//
//

#ifndef SpellSystem_hpp
#define SpellSystem_hpp

struct spellCompareID
{
	bool operator()(const Spell* left, const Spell* right) const;
};

class SpellSystem
{
public:
	friend class GSpace;

	static constexpr bool logSpells = false;

	SpellSystem(GSpace* gspace);
	~SpellSystem();

	local_shared_ptr<Spell> cast(const SpellDesc* desc, GObject* caster);

    void stopSpell(local_shared_ptr<Spell> spell);
    void stopSpell(Spell* spell);
protected:
	void applyRemove(Spell* spell);
	void applyRemovals();
	void stopObjectSpells(GObject* obj);
	void update();
	
	list<Spell*> additions;
	//This is a set in order to ensure against double-deletion
	set<Spell*> removals;

	set<local_shared_ptr<Spell>> spells;
	set<Spell*, spellCompareID> updateSpells;
	multimap<GObject*, Spell*> objectSpells;

	GSpace* gspace;
	unsigned int nextID = 1;
};

#endif
