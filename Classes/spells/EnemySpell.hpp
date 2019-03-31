//
//  EnemySpell.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef EnemySpell_hpp
#define EnemySpell_hpp

#include "object_ref.hpp"
#include "Spell.hpp"

class TeleportPad;
class Torch;

class Teleport : public Spell {
public:
	static const string name;
	static const string description;
	static const float cost;

	Teleport(GObject* caster);
	inline virtual ~Teleport() {}

	GET_DESC(Teleport)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	vector<object_ref<TeleportPad>> targets;
	object_ref<TeleportPad> toUse;
};

class TorchDarkness : public Spell
{
public:
	static const string name;
	static const string description;

	static const SpaceFloat radius;
	static const float effectMagnitude;

	TorchDarkness(GObject* caster);
	inline virtual ~TorchDarkness() {}

	GET_DESC(TorchDarkness)
	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
};

#endif /* EnemySpell_hpp */
