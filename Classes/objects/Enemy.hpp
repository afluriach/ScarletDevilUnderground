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

class Enemy : virtual public Agent
{
public:
	Enemy(collectible_id drop_id);
	inline virtual ~Enemy() {}

	void runDamageFlicker();

	virtual bool hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect);
	virtual void onRemove();

	virtual inline GType getType() const { return GType::enemy; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }

protected:
	collectible_id drop_id = collectible_id::nil;
};

#endif /* Enemy_hpp */
