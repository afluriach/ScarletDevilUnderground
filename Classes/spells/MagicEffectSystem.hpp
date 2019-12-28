//
//  MagicEffectSystem.hpp
//  Koumachika
//
//  Created by Toni on 6/24/19.
//
//

#ifndef MagicEffectSystem_hpp
#define MagicEffectSystem_hpp

#include "MagicEffect.hpp"

struct timedEntry
{
	unsigned int endFrame;
	MagicEffect* effect;

	bool operator>(const timedEntry& rhs) const;
};

struct effectCompareID
{
	inline bool operator()(const MagicEffect* left, const MagicEffect* right) const
	{
		return left->id < right->id;
	}
};

class MagicEffectSystem
{
public:
	friend class GSpace;
	friend class MagicEffect;

	MagicEffectSystem(GSpace* gspace);

	unsigned int applyEffect(GObject* target, shared_ptr<MagicEffectDescriptor> effect, float magnitude, float length);

	void addEffect(shared_ptr<MagicEffect> effect);
	void removeEffect(shared_ptr<MagicEffect> effect);
	void removeEffect(unsigned int id);
	void removeObjectEffects(GObject* obj);

	shared_ptr<MagicEffect> getByID(unsigned int id);
	bool hasScriptedEffect(GObject* obj, string clsName);

	template<class T>
	inline bool hasMagicEffect(GObject* obj)
	{
		auto it = effectObjects.find(obj);
		if (it == effectObjects.end()) return false;

		for (auto entry : it->second) {
			if (dynamic_cast<T*>(entry)) {
				return true;
			}
		}
		return false;
	}
protected:
	void applyAdd();
	void applyRemove();
	void update();
	//Examined and updates the timed removals heap, removing entries from the heap
	//if applicable, and added timed to magicEffectsToRemove.
	void processTimedRemovals();

	bool isValidConfig(MagicEffect* effect);

	map<GObject*, set<MagicEffect*, effectCompareID>> effectObjects;
	
	//Store all timed effects by their ending time as frame number.
	priority_queue<timedEntry, vector<timedEntry>, greater<timedEntry>> timedRemovals;

	set<MagicEffect*, effectCompareID> updateEffects;

	map<unsigned int, shared_ptr<MagicEffect>> magicEffects;
	list<shared_ptr<MagicEffect>> magicEffectsToAdd;
	list<unsigned int> magicEffectsToRemove;

	GSpace* gspace;
	unsigned int nextID = 1;
};

#endif
