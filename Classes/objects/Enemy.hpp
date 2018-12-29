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

class Enemy : virtual public Agent, public RegisterUpdate<Enemy>
{
public:
	Enemy(collectible_id drop_id);

	void onTouchPlayer(Player* target);
	void endTouchPlayer();

	void runDamageFlicker();
	void update();

	virtual void onRemove();

	virtual AttributeMap touchEffect();

	virtual inline GType getType() const { return GType::enemy; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }

protected:
	object_ref<Player> touchTarget;
	collectible_id drop_id = collectible_id::nil;
};

#endif /* Enemy_hpp */
