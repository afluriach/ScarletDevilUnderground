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

class MagicEffectSystem
{
public:

	friend class GSpace;

	MagicEffectSystem(GSpace* gspace);

	void addEffect(MagicEffect* effect);
	void removeEffect(MagicEffect* effect);
	void removeObjectEffects(GObject* obj);

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

	map<GObject*, list<MagicEffect*>> effectObjects;
	
	//Store all timed effects by their ending time as frame number.
	priority_queue<timedEntry, vector<timedEntry>, greater<timedEntry>> timedRemovals;

	list<MagicEffect*> updateEffects;

	//need to store timed magic effects sorted by ending time
	//need to store magic effects that requie an update sepearately
	list<MagicEffect*> magicEffects;
	list<MagicEffect*> magicEffectsToAdd;
	list<MagicEffect*> magicEffectsToRemove;

	GSpace* gspace;
};

#endif
