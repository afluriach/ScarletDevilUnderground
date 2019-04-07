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

class GreenFairyPowerAttack : public Spell
{
public:
	static const string name;
	static const string description;

	GreenFairyPowerAttack(GObject* caster, int waveCount, int bulletsPerWave, SpaceFloat interval);
	inline virtual ~GreenFairyPowerAttack() {}

	int spawn();

	GET_DESC(GreenFairyPowerAttack);
	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	SpaceFloat timer = 0.0;
	SpaceFloat interval;
	int wavesFired = 0;
	int waveCount;
	int bulletsPerWave;
};

#endif /* EnemySpell_hpp */
