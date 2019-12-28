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

class FireStarburst : public Spell {
public:
	static const string name;
	static const string description;

	static constexpr float bulletSpeed = 6.0f;

	FireStarburst(GObject* caster);
	inline virtual ~FireStarburst() {}

	GET_DESC(FireStarburst)

	virtual void update();
protected:
	shared_ptr<bullet_properties> bulletProps;
};

class FlameFence : public Spell {
public:
	static const string name;
	static const string description;

	FlameFence(GObject* caster);
	inline virtual ~FlameFence() {}

	GET_DESC(FlameFence)
	virtual void init();
	virtual void end();
protected:
	vector<gobject_ref> bullets;
	shared_ptr<bullet_properties> bulletProps;
};

#endif /* PatchouliSpell_hpp */
