//
//  spell_types.hpp
//  Koumachika
//
//  Created by Toni on 3/31/19.
//
//

#ifndef spell_types_hpp
#define spell_types_hpp

struct spell_cost
{
	float initial_mp = 0.0f;
	float initial_stamina = 0.0f;

	//units per second
	float ongoing_mp = 0.0f;
	float ongoing_stamina = 0.0f;

	static spell_cost none();
	static spell_cost initialMP(float mp);
	static spell_cost initialStamina(float stamina);
	static spell_cost ongoingMP(float mp);
};

struct spell_params
{
	spell_params(SpaceFloat length = 0.0, SpaceFloat updateInterval = 0.0, spell_cost cost = spell_cost{});

	SpaceFloat length = 0.0;
	SpaceFloat updateInterval = 0.0;
	spell_cost cost = {};
};

#endif /* spell_types_hpp */
