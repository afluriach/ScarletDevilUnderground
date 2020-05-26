//
//  PlayerSpell.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef PlayerSpell_hpp
#define PlayerSpell_hpp

#include "Spell.hpp"
#include "spell_types.hpp"
#include "SpellImpl.hpp"

class PlayerBatMode : public ApplySelfEffect{
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	PlayerBatMode(GObject* caster, const SpellDesc* desc, unsigned int id);
	~PlayerBatMode();

	GET_DESC(PlayerBatMode)
};

class LavaeteinnSpell : public MeleeAttack
{
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	static const SpaceFloat length;
	static const SpaceFloat angleWidth;
	static const SpaceFloat angular_speed;

	static const int bulletSpawnCount;

	LavaeteinnSpell(GObject* caster, const SpellDesc* desc, unsigned int id);
	~LavaeteinnSpell();

	GET_DESC(LavaeteinnSpell);
};

class PlayerCounterClock : public CirclingBullets {
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	static const SpaceFloat offset;
	static const SpaceFloat angular_speed;

	PlayerCounterClock(GObject* caster, const SpellDesc* desc, unsigned int id);
	~PlayerCounterClock();

	GET_DESC(PlayerCounterClock)
};

class PlayerIceShield : public CirclingBullets {
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	static constexpr size_t bulletCount = 8;
	static const SpaceFloat speed;
	static const SpaceFloat angularSpeed;

	static const SpaceFloat distance;
	static const SpaceFloat circumference;
	static const SpaceFloat inv_circumference;

	PlayerIceShield(GObject* caster, const SpellDesc* desc, unsigned int id);
	~PlayerIceShield();

	GET_DESC(PlayerIceShield)
};

#endif /* PlayerSpell_hpp */
