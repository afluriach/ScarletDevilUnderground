//
//  EnemySpell.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef EnemySpell_hpp
#define EnemySpell_hpp

#include "Attributes.hpp"
#include "Spell.hpp"

class Torch;

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

class BlueFairyBomb : public Spell
{
public:
	static const DamageInfo damage;
	static const SpaceFloat length;
	static const SpaceFloat radius;
	static const SpaceFloat angularSpeed;

	BlueFairyBomb(GObject* caster);
	inline virtual ~BlueFairyBomb() {}

	GET_DESC(BlueFairyBomb);
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	SpriteID sprite = 0;
	SpaceFloat timer = 0.0;
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
