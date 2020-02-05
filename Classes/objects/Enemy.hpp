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
#include "Collectibles.hpp"

class enemy_properties : public agent_properties
{
public:
	inline enemy_properties() {}

	string firepattern;
	string collectible;

	DamageInfo touchEffect;
};

class Enemy : public Agent
{
public:
	Enemy(
		GSpace* space,
		ObjectIDType id,
		const agent_attributes& attr,
		local_shared_ptr<enemy_properties> props
	);

	inline virtual ~Enemy() {}

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
