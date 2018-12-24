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

class Enemy : virtual public Agent, public RegisterUpdate<Enemy>
{
public:
	Enemy();

	void onTouchPlayer(Player* target);
	void endTouchPlayer();

	void runDamageFlicker();
	void update();

	virtual inline GType getType() const { return GType::enemy; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }

protected:
	object_ref<Player> touchTarget;
};

#endif /* Enemy_hpp */
