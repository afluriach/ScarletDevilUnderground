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

//Apply power drain and deactive spell if applicable
class PlayerSpell : virtual public Spell {
public:
	inline PlayerSpell() {}
	inline virtual ~PlayerSpell() {}

	virtual void init();
	virtual void update();
	virtual void end();

	virtual SpaceFloat getLength() const = 0;
protected:
	SpaceFloat timeInSpell = 0.0;
};

class PlayerBatMode : virtual public Spell, public PlayerSpell{
public:
	static const string name;
	static const string description;

	PlayerBatMode(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
    virtual void init();
    virtual void end();

	inline virtual SpaceFloat getLength() const { return 5.0; }
};

class PlayerCounterClock : virtual public Spell, public PlayerSpell {
public:
	static const string name;
	static const string description;

	static const SpaceFloat offset;
	static const SpaceFloat angular_speed;

	PlayerCounterClock(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
	virtual void init();
	virtual void update();
	virtual void end();

	inline virtual SpaceFloat getLength() const { return 5.0; }
protected:
	array<object_ref<FlandreCounterClockBullet>, 4> bullets;
	SpaceFloat angular_pos = 0.0;
};

class PlayerDarkMist : virtual public Spell, public PlayerSpell {
public:
	static const string name;
	static const string description;

	PlayerDarkMist(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
	virtual void init();
	virtual void end();

	inline virtual SpaceFloat getLength() const { return 7.5; }
};

class PlayerIceShield : virtual public Spell, public PlayerSpell {
public:
	static const string name;
	static const string description;

	static constexpr size_t bulletCount = 8;
	static const SpaceFloat speed;
	static const SpaceFloat distance;
	static const SpaceFloat circumference;
	static const SpaceFloat inv_circumference;

	PlayerIceShield(GObject* caster, const ValueMap& args, SpellDesc* descriptor);
	virtual void init();
	virtual void update();
	virtual void end();

	inline virtual SpaceFloat getLength() const { return 5.0; }
protected:
	array<object_ref<CirnoIceShieldBullet>, bulletCount> bullets;
	SpaceFloat crntAngle = 0.0;
};

#endif /* PlayerSpell_hpp */
