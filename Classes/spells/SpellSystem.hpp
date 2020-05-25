//
//  SpellSystem.hpp
//  Koumachika
//
//  Created by Toni on 12/30/19.
//
//

#ifndef SpellSystem_hpp
#define SpellSystem_hpp

class Bullet;
class Spell;

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

	unsigned int cast(const SpellDesc* desc, GObject* caster);

	void stopSpell(unsigned int id);
	bool isSpellActive(unsigned int id);
	bool isSpellActive(const SpellDesc* desc, GObject* caster);

	void onRemove(unsigned int id, Bullet* b);
protected:
	void applyRemove(Spell* spell);
	void applyRemovals();
	void stopObjectSpells(GObject* obj);
	void update();
	
	list<Spell*> additions;
	//This is a set in order to ensure against double-deletion
	set<Spell*> removals;

	map<unsigned int, Spell*> spells;
	set<Spell*, spellCompareID> updateSpells;
	multimap<GObject*, Spell*> objectSpells;

	GSpace* gspace;
	unsigned int nextID = 1;
};

#endif
