//
//  MarisaSpell.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef MarisaSpell_hpp
#define MarisaSpell_hpp

#include "Spell.hpp"

//A somewhat conical, but mostly focused attack.
//Most of the shots to be within half of the cone width.
//Speed variation: maybe a factor of two.
//Size variation, similarly, and the two should be inversely correlated.
class StarlightTyphoon : public Spell {
public:
	static const string name;
	static const string description;

	static const SpaceFloat width;
	static const SpaceFloat duration;
	static const SpaceFloat offset;

	static const unsigned int count;

	static const vector<string> colors;

	StarlightTyphoon(GObject* caster);
	inline virtual ~StarlightTyphoon() {}

	GET_DESC(StarlightTyphoon)
	virtual void init();
	virtual void update();
	virtual void end();

	void fire();
protected:
	SpaceFloat elapsed = 0.0;
	SpaceFloat shotsPerFrame;
	SpaceFloat accumulator = 0.0;
	SpaceFloat angle;
};

#endif /* MarisaSpell_hpp */
