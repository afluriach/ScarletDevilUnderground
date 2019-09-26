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
	shared_ptr<MagicEffect> effect;

	bool operator>(const timedEntry& rhs) const;
};

class MagicEffectSystem
{
public:

	friend class GSpace;

	MagicEffectSystem(GSpace* gspace);

	void addEffect(shared_ptr<MagicEffect> effect);
	void removeEffect(shared_ptr<MagicEffect> effect);
	void removeObjectEffects(GObject* obj);

	template<class T>
	inline bool hasMagicEffect(GObject* obj)
	{
		auto it = effectObjects.find(obj);
		if (it == effectObjects.end()) return false;

		for (auto entry : it->second) {
			if (dynamic_cast<T*>(entry.get())) {
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

	bool isValidConfig(shared_ptr<MagicEffect> effect);

	map<GObject*, list<shared_ptr<MagicEffect>>> effectObjects;
	
	//Store all timed effects by their ending time as frame number.
	priority_queue<timedEntry, vector<timedEntry>, greater<timedEntry>> timedRemovals;

	list<shared_ptr<MagicEffect>> updateEffects;

	//need to store timed magic effects sorted by ending time
	//need to store magic effects that requie an update sepearately
	list<shared_ptr<MagicEffect>> magicEffects;
	list<shared_ptr<MagicEffect>> magicEffectsToAdd;
	list<shared_ptr<MagicEffect>> magicEffectsToRemove;

	GSpace* gspace;
};

#endif
