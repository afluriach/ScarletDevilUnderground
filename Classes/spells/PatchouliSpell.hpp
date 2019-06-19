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
	static const float cost;

	static constexpr float bulletSpeed = 6.0f;

	FireStarburst(GObject* caster);
	inline virtual ~FireStarburst() {}

	GET_DESC(FireStarburst)
	inline virtual void init() {}
	inline virtual void end() {}

	inline virtual float interval() const { return 0.5f; }
	virtual void runPeriodic();
protected:
	shared_ptr<bullet_properties> bulletProps;
};

class FlameFence : public Spell {
public:
	static const string name;
	static const string description;
	static const float cost;

	FlameFence(GObject* caster);
	inline virtual ~FlameFence() {}

	GET_DESC(FlameFence)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	vector<gobject_ref> bullets;
	shared_ptr<bullet_properties> bulletProps;
};

class Whirlpool1 : public Spell {
public:
	static const string name;
	static const string description;
	static const float cost;

	static const int shotsPerSecond;

	static const SpaceFloat angularSpeed;
	static const SpaceFloat angularOffset;
	static const SpaceFloat bulletSpeed;

	Whirlpool1(GObject* caster);
	inline virtual ~Whirlpool1() {}

	GET_DESC(Whirlpool1)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	SpaceFloat shotTimer = 0.0;
	shared_ptr<bullet_properties> bulletProps;
};

class Whirlpool2 : public Spell {
public:
	static const string name;
	static const string description;
	static const float cost;

	static const int shotsPerSecond;

	static const SpaceFloat angularSpeed;
	static const SpaceFloat angularOffset;
	static const SpaceFloat bulletSpeed;

	Whirlpool2(GObject* caster);
	inline virtual ~Whirlpool2() {}

	GET_DESC(Whirlpool2)
	virtual void init();
	virtual void update();
	virtual void end();
protected:
	SpaceFloat shotTimer = 0.0;
	shared_ptr<bullet_properties> bulletProps;
};


#endif /* PatchouliSpell_hpp */
