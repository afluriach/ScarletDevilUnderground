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

	virtual void onRemove();

	void runDamageFlicker();

	virtual DamageInfo touchEffect() const;
	virtual bool hit(DamageInfo damage, SpaceVect n);
protected:
	local_shared_ptr<enemy_properties> props;
};

#endif /* Enemy_hpp */
