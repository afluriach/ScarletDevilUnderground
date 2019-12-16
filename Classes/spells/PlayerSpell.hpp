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

class BulletImpl;

class PlayerBatMode : public Spell{
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	PlayerBatMode(GObject* caster);
	inline virtual ~PlayerBatMode() {}

	GET_DESC(PlayerBatMode)
    virtual void init();
    virtual void end();
};

class LavaeteinnSpell : public Spell
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

	LavaeteinnSpell(GObject* caster);
	inline virtual ~LavaeteinnSpell() {}

	GET_DESC(LavaeteinnSpell);
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	gobject_ref lavaeteinnBullet;
	SpaceFloat fireTimer;
	SpaceFloat angularPos;
	SpaceFloat speedScale;
};

class PlayerCounterClock : public Spell {
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	static const SpaceFloat offset;
	static const SpaceFloat angular_speed;

	PlayerCounterClock(GObject* caster);
	inline virtual ~PlayerCounterClock() {}

	GET_DESC(PlayerCounterClock)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	array<object_ref<BulletImpl>, 4> bullets;
	SpaceFloat angular_pos = 0.0;
};

class PlayerScarletRose : public Spell {
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	static const SpaceFloat fireInterval;
	static const int fireCount;

	//parametric / angle scale - at 1.0, it takes 2pi seconds
	//to trace the entire graph
	static const SpaceFloat W;
	//magnitude scale
	static const SpaceFloat A;
	//frequency, number of petals
	static const SpaceFloat B;

	static SpaceVect parametric_motion(SpaceFloat t);

	PlayerScarletRose(GObject* caster);

	GET_DESC(PlayerScarletRose)
	virtual void update();
	virtual void end();

	const SpaceVect origin;
	unordered_set<gobject_ref> bullets;
	SpaceFloat timer = 0.0;
	int launchCount = 0;
protected:
	shared_ptr<bullet_properties> props;
};

class PlayerDarkMist : public Spell {
public:
	static const string name;
	static const string description;
	static const spell_cost cost;

	PlayerDarkMist(GObject* caster);

	GET_DESC(PlayerDarkMist)
	virtual void init();
	virtual void end();
};

class PlayerIceShield : public Spell {
public:
	static const string name;
	static const string description;
	static const string icon;
	static const spell_cost cost;

	static constexpr size_t bulletCount = 8;
	static const SpaceFloat speed;
	static const SpaceFloat distance;
	static const SpaceFloat circumference;
	static const SpaceFloat inv_circumference;

	PlayerIceShield(GObject* caster);
	inline virtual ~PlayerIceShield() {}

	GET_DESC(PlayerIceShield)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	array<object_ref<BulletImpl>, bulletCount> bullets;
	shared_ptr<bullet_properties> props;
	SpaceFloat crntAngle = 0.0;
};

#endif /* PlayerSpell_hpp */
