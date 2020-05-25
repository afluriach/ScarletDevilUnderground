//
//  EnemySpell.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef EnemySpell_hpp
#define EnemySpell_hpp

#include "Spell.hpp"

class BlueFairyBomb : public Spell
{
public:
	static const string name;
	static const string description;

	static const DamageInfo damage;
	static const SpaceFloat length;
	static const SpaceFloat radius;
	static const SpaceFloat angularSpeed;

	BlueFairyBomb(GObject* caster, const SpellDesc* desc, unsigned int id);
	inline virtual ~BlueFairyBomb() {}

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	SpriteID sprite = 0;
};

#endif /* EnemySpell_hpp */
