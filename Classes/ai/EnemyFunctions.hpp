//
//  EnemyFunctions.hpp
//  Koumachika
//
//  Created by Toni on 4/8/19.
//
//

#ifndef EnemyFunctions_hpp
#define EnemyFunctions_hpp

#include "AI.hpp"

class SpellDesc;

namespace ai{

class AimAtTarget;
class Cast;
class FireAtTarget;
class Flank;
class FollowPath;
class RumiaDSD2;

class BlueFairyPowerAttack : public ai::Function {
public:
	static const SpaceFloat cooldown;
	static const SpaceFloat triggerLength;
	static const SpaceFloat triggerDist;
	static const float cost;

	BlueFairyPowerAttack(StateMachine* fsm);
	~BlueFairyPowerAttack();

	virtual update_return update();
protected:
	SpaceFloat accumulator = 0.0;
	SpaceFloat timer = cooldown;
	unsigned int spellID = 0;
};

} //end NS

#endif /* EnemyFunctions_hpp */
