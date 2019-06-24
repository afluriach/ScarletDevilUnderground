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

class MagicEffectSystem
{
public:
	friend class GSpace;

	MagicEffectSystem();

	void addEffect(shared_ptr<MagicEffect> effect);
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

	map<GObject*, list<shared_ptr<MagicEffect>>> effectObjects;

	//need to store timed magic effects sorted by ending time
	//need to store magic effects that requie an update sepearately
	list<shared_ptr<MagicEffect>> magicEffects;
	list<shared_ptr<MagicEffect>> magicEffectsToAdd;
	list<shared_ptr<MagicEffect>> magicEffectsToRemove;
};

#endif
