//
//  PatchouliSpell.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef PatchouliSpell_hpp
#define PatchouliSpell_hpp

#include "Spell.hpp"

class FireStarburst : public PeriodicSpell {
public:
	static const string name;
	static const string description;

	static constexpr float bulletSpeed = 6.0f;

	STANDARD_CONS(FireStarburst)
		no_op(init);
	inline float interval() const { return 0.5f; }
	void runPeriodic();
	no_op(end);
};

class FlameFence : public Spell {
public:
	static const string name;
	static const string description;

	STANDARD_CONS(FlameFence)
		FlameFence(GObject* caster);

	void init();
	void update();
	void end();
protected:
	vector<gobject_ref> bullets;
};

class Whirlpool1 : public Spell {
public:
	static const string name;
	static const string description;

	static const int shotsPerSecond;

	static const SpaceFloat angularSpeed;
	static const SpaceFloat angularOffset;
	static const SpaceFloat bulletSpeed;

	STANDARD_CONS(Whirlpool1)
		Whirlpool1(GObject* caster);

	void init();
	void update();
	void end();
protected:
	SpaceFloat shotTimer = 0.0;
};

class Whirlpool2 : public Spell {
public:
	static const string name;
	static const string description;

	static const int shotsPerSecond;

	static const SpaceFloat angularSpeed;
	static const SpaceFloat angularOffset;
	static const SpaceFloat bulletSpeed;

	STANDARD_CONS(Whirlpool2)
		Whirlpool2(GObject* caster);

	void init();
	void update();
	void end();
protected:
	SpaceFloat shotTimer = 0.0;
};


#endif /* PatchouliSpell_hpp */
