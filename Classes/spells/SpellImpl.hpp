//
//  SpellImpl.hpp
//  Koumachika
//
//  Created by Toni on 2/23/20.
//
//

#ifndef SpellImpl_hpp
#define SpellImpl_hpp

#include "Spell.hpp"

class ScriptedSpell : public Spell {
public:
	static spell_params getParams(string clsName);

	ScriptedSpell(GObject* caster, const SpellDesc* desc, unsigned int id, string clsName);
	virtual ~ScriptedSpell();

	virtual void init();
	virtual void update();
	virtual void end();
	virtual void onBulletRemove(Bullet* b);
protected:
	string clsName;
	sol::table obj;
};

class ApplySelfEffect : public Spell
{
public:
	ApplySelfEffect(
		GObject* caster,
		const SpellDesc* desc,
		unsigned int id,
		spell_params params,
		const MagicEffectDescriptor* effect
	);
	~ApplySelfEffect();

	virtual void init();
	virtual void end();
protected:
	const MagicEffectDescriptor* effect;
	unsigned int effectID = 0;
};

class MeleeAttack : public Spell
{
public:
	MeleeAttack(
		GObject* caster,
		const SpellDesc* desc,
		unsigned int id,
		spell_params params,
		melee_params melee
	);

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	melee_params melee;

	SpaceFloat fireTimer;
	SpaceFloat angularPos;
	SpaceFloat angular_speed;
	gobject_ref bullet;
};

#endif 
