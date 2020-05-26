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

	inline float get_initial_mp() const { return initial_mp; }
	inline float get_initial_stamina() const { return initial_stamina; }
	inline float get_ongoing_mp() const { return ongoing_mp; }
	inline float get_ongoing_stamina() const { return ongoing_stamina; }

	inline void set_initial_mp(float v) { initial_mp = v; }
	inline void set_initial_stamina(float v) { initial_stamina = v; }
	inline void set_ongoing_mp(float v) { ongoing_mp = v; }
	inline void set_ongoing_stamina(float v) { ongoing_stamina = v; }

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

struct circling_bullets_params
{
	local_shared_ptr<bullet_properties> bullet;

	SpaceFloat distance = 0.0;
	SpaceFloat angularSpeed = 0.0;
	int bulletCount = 0;
};

struct melee_params
{
	SpaceFloat length = 0.0;
	SpaceFloat swingDist = 0.0;
	SpaceFloat sideAngleWidth = 0.0;

	local_shared_ptr<bullet_properties> melee;
	local_shared_ptr<bullet_properties> bullet;
	SpaceFloat launchDist = 0.0;
	int fireCount = 0;
};

#endif /* spell_types_hpp */
