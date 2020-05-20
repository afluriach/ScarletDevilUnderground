//
//  Enemy.hpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#ifndef Enemy_hpp
#define Enemy_hpp

#include "Agent.hpp"

class enemy_properties : public agent_properties
{
public:
	inline enemy_properties() {}

	string firepattern;
	string collectible;

	DamageInfo touchEffect;

	inline virtual type_index getType() const { return typeid(*this); }
};

class Enemy : public Agent
{
public:
	Enemy(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<enemy_properties> props
	);

	~Enemy();

	virtual void init();
	virtual void onRemove();

	void runDamageFlicker();

	void loadEffects();

	virtual DamageInfo touchEffect() const;
	virtual bool hit(DamageInfo damage, SpaceVect n);
protected:
	local_shared_ptr<enemy_properties> props;
};

#endif /* Enemy_hpp */
