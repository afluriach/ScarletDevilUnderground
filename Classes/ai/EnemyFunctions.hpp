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

	virtual update_return update();
protected:
	SpaceFloat accumulator = 0.0;
	SpaceFloat timer = cooldown;
	unsigned int spellID = 0;
};

class MarisaCollectMain : public Function {
public:
	inline MarisaCollectMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(MarisaCollectMain)
protected:
	shared_ptr<FollowPath> moveFunction;
};

class MarisaForestMain : public Function {
public:
	inline MarisaForestMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(MarisaForestMain)
protected:
	shared_ptr<AimAtTarget> aimFunction;
	shared_ptr<Cast> castFunction;
};

class ReimuYinYangOrbs : public Function
{
public:
	static constexpr int orbCount = 4;

	ReimuYinYangOrbs(StateMachine* fsm);

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
protected:
	array<gobject_ref, orbCount> orbs;
	bool active = false;
};

class RumiaMain1 : public Function
{
public:
	static const SpaceFloat dsdDistMargin;
	static const SpaceFloat dsdLength;
	static const SpaceFloat dsdCooldown;
	static const float dsdCost;

	RumiaMain1(StateMachine* fsm, gobject_ref target);

	virtual void onEnter();
	virtual void onReturn();
	virtual update_return update();
	virtual void onExit();
	FuncGetName(RumiaMain1);
protected:
	shared_ptr<FireAtTarget> fireFunction;
	shared_ptr<Flank> moveFunction;
	gobject_ref target;
	SpaceFloat dsdTimer = 0.0;
	size_t intervalIdx = 0;
};

class RumiaMain2 : public Function
{
public:
	RumiaMain2(StateMachine* fsm, gobject_ref target);

	virtual void onEnter();
	virtual update_return update();
protected:
	shared_ptr<FireAtTarget> fire;
	shared_ptr<Flank> flank;
	shared_ptr<RumiaDSD2> dsd;

	gobject_ref target;
};

class RumiaDSD2 : public ai::Function
{
public:
	inline RumiaDSD2(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();
	FuncGetName(RumiaDSD2);
protected:
	SpaceFloat timer = 0.0;
	size_t intervalIdx = 0;
};

class SakuyaMain : public Function {
public:
	inline SakuyaMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(SakuyaMain)
};

class IllusionDash : public Function {
public:
	inline IllusionDash(StateMachine* fsm) : Function(fsm) {}

	IllusionDash(StateMachine* fsm, SpaceVect _target);
	IllusionDash(StateMachine* fsm, const ValueMap& args);
	static const SpaceFloat scale;
	static const SpaceFloat opacity;
	static const SpaceFloat speed;

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(IllusionDash)
protected:
	SpaceVect target;
};

} //end NS

#endif /* EnemyFunctions_hpp */
