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
};

class BatMain : public Function {
public:
	inline BatMain(StateMachine* fsm, gobject_ref target) : Function(fsm), target(target) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(BatMain)
protected:
	gobject_ref target = nullptr;
};

class FacerMain : public Function {
public:
	inline FacerMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
	FuncGetName(FacerMain)
protected:
	gobject_ref target = nullptr;
};

class FollowerMain : public Function {
public:
	inline FollowerMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
	FuncGetName(FollowerMain)
protected:
	gobject_ref target = nullptr;
};

class MarisaCollectMain : public Function {
public:
	inline MarisaCollectMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(MarisaCollectMain)
};

class MarisaForestMain : public Function {
public:
	inline MarisaForestMain(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(MarisaForestMain)
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
	gobject_ref target;
	SpaceFloat dsdTimer = 0.0;
	size_t intervalIdx = 0;
};

class RumiaMain2 : public Function
{
public:
	RumiaMain2(StateMachine* fsm, gobject_ref target);

	virtual update_return update();
protected:
	shared_ptr<Function> flankThread;
	gobject_ref target;
};

class RumiaDSD2 : public ai::Function
{
public:
	inline RumiaDSD2(StateMachine* fsm) : Function(fsm) {}

	static const vector<double_pair> demarcationSizeIntervals;

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

class SakuyaNPC1 : public Function {
public:
	inline SakuyaNPC1(StateMachine* fsm) : Function(fsm) {}

	virtual void onEnter();
	virtual update_return update();
	FuncGetName(SakuyaMain)
};

class StalkerMain : public Function {
public:
	inline StalkerMain(StateMachine* fsm) : Function(fsm) {}

	virtual bool onEvent(Event event);
	virtual update_return update();
	virtual event_bitset getEvents();
	FuncGetName(StalkerMain)

	void applyTeleport();
};

} //end NS

#endif /* EnemyFunctions_hpp */
