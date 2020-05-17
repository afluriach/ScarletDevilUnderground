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

class RumiaMain2 : public Function
{
public:
	RumiaMain2(StateMachine* fsm, gobject_ref target);
	~RumiaMain2();

	virtual void onEnter();
	virtual update_return update();
protected:
	local_shared_ptr<FireAtTarget> fire;
	local_shared_ptr<Flank> flank;
	local_shared_ptr<RumiaDSD2> dsd;

	gobject_ref target;
};

class RumiaDSD2 : public ai::Function
{
public:
	inline RumiaDSD2(StateMachine* fsm) : Function(fsm) {}
	~RumiaDSD2();

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();
	FuncGetName(RumiaDSD2);
protected:
	SpaceFloat timer = 0.0;
	size_t intervalIdx = 0;
};

} //end NS

#endif /* EnemyFunctions_hpp */
