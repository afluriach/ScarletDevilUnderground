//
//  SpellImpl.hpp
//  Koumachika
//
//  Created by Toni on 2/23/20.
//
//

#ifndef SpellImpl_hpp
#define SpellImpl_hpp

class ScriptedSpell : public Spell {
public:
	static spell_params getParams(string clsName);

	ScriptedSpell(GObject* caster, const SpellDesc* desc, string clsName);
	virtual ~ScriptedSpell();

protected:
	virtual void init();
	virtual void update();
	virtual void end();
	virtual void onBulletRemove(Bullet* b);
	string clsName;
	sol::table obj;
};

class ApplySelfEffect : public Spell
{
public:
	ApplySelfEffect(
		GObject* caster,
		const SpellDesc* desc,
		const MagicEffectDescriptor* effect
	);
	~ApplySelfEffect();

protected:
	virtual void init();
	virtual void end();
	const MagicEffectDescriptor* effect;
	unsigned int effectID = 0;
};

class MeleeAttack : public Spell
{
public:
	MeleeAttack(
		GObject* caster,
		const SpellDesc* desc,
		melee_params melee
	);

protected:
	virtual void init();
	virtual void update();
	virtual void end();
	melee_params melee;

	SpaceFloat fireTimer;
	SpaceFloat angularPos;
	SpaceFloat angular_speed;
	gobject_ref bullet;
};

class CirclingBullets : public Spell
{
public:
	CirclingBullets(
		GObject* caster,
		const SpellDesc* desc,
		circling_bullets_params _params
	);

protected:
	virtual void init();
	virtual void update();
	virtual void end();
	circling_bullets_params params;
	vector<gobject_ref> bullets;
	SpaceFloat angularPos = 0.0;
};

#endif 
