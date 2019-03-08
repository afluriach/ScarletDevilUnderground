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

class CirnoIceShieldBullet;
class FlandreCounterClockBullet;

//Apply power drain and deactive spell if applicable
class PlayerSpell : public Spell {
public:
	inline PlayerSpell(GObject* caster) : Spell(caster) {}
	inline virtual ~PlayerSpell() {}

	virtual void init();
	virtual void update();
	virtual void end();

	virtual SpaceFloat getLength() const = 0;
protected:
	SpaceFloat timeInSpell = 0.0;
};

class PlayerBatMode : public PlayerSpell{
public:
	static const string name;
	static const string description;

	PlayerBatMode(GObject* caster);
	inline virtual ~PlayerBatMode() {}

	GET_DESC(PlayerBatMode)
    virtual void init();
    virtual void end();

	inline virtual SpaceFloat getLength() const { return 5.0; }
};

class PlayerCounterClock : public PlayerSpell {
public:
	static const string name;
	static const string description;

	static const SpaceFloat offset;
	static const SpaceFloat angular_speed;

	PlayerCounterClock(GObject* caster);
	inline virtual ~PlayerCounterClock() {}

	GET_DESC(PlayerCounterClock)
	virtual void init();
	virtual void update();
	virtual void end();

	inline virtual SpaceFloat getLength() const { return 5.0; }
protected:
	array<object_ref<FlandreCounterClockBullet>, 4> bullets;
	SpaceFloat angular_pos = 0.0;
};

class PlayerDarkMist : public PlayerSpell {
public:
	static const string name;
	static const string description;

	PlayerDarkMist(GObject* caster);

	GET_DESC(PlayerDarkMist)
	virtual void init();
	virtual void end();

	inline virtual SpaceFloat getLength() const { return 7.5; }
};

class PlayerIceShield : public PlayerSpell {
public:
	static const string name;
	static const string description;

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

	inline virtual SpaceFloat getLength() const { return 5.0; }
protected:
	array<object_ref<CirnoIceShieldBullet>, bulletCount> bullets;
	SpaceFloat crntAngle = 0.0;
};

#endif /* PlayerSpell_hpp */
