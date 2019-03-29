//
//  SakuyaSpell.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef SakuyaSpell_hpp
#define SakuyaSpell_hpp

#include "Spell.hpp"

class IllusionDialDagger;

class IllusionDial : public Spell
{
public:
	static const string name;
	static const string description;
	static const float cost;

	static const float radius;
	static const float arc_start;
	static const float arc_end;
	static const float arc_width;
	static const float arc_spacing;
	static const float angular_speed;

	static const float max_angle_margin;
	static const float min_fire_interval;

	static const int count;

	vector<object_ref<IllusionDialDagger>> bullets;
	vector<bool> launch_flags;
	unsigned int framesSinceLastFire = 0;

	IllusionDial(GObject* caster);
	inline virtual ~IllusionDial() {}

	virtual void update();

	GET_DESC(IllusionDial)
	virtual void init();
	virtual void end();
};



#endif /* SakuyaSpell_hpp */
