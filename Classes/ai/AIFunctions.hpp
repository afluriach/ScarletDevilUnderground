//
//  AIFunctions.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef AIFunctions_hpp
#define AIFunctions_hpp

#include "AI.hpp"

namespace ai{

class OnDetect : public Function {
public:
	OnDetect(StateMachine* fsm, GType type, AITargetFunctionGenerator gen);

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
protected:
	shared_ptr<Thread> thread;
	AITargetFunctionGenerator gen;
	GType type;
};

class WhileDetect : public Function {
public:
	WhileDetect(StateMachine* fsm, GType type, AITargetFunctionGenerator gen);

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
protected:
	shared_ptr<Thread> thread;
	AITargetFunctionGenerator gen;
	GType type;
};

class CompositeFunction : public Function {
public:
	//create empty
	CompositeFunction(StateMachine* fsm);

	template<class FuncCls, typename... Params>
	inline void addFunction(Params... params)
	{
		addFunction(make_shared<FuncCls>(fsm, params...));
	}

	virtual void onEnter();
	virtual update_return update();
	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
	virtual void onExit();
	virtual lock_mask getLockMask();
	virtual string getName();

	void addFunction(shared_ptr<Function> f);
	void removeFunction(shared_ptr<Function> f);
protected:
	list<shared_ptr<Function>> functions;
	event_bitset events;
	bool hasInit = false;
};

class BossFightHandler : public Function {
public:
	BossFightHandler(StateMachine* fsm, string startDialog, string endDialog);

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
protected:
	string startDialog, endDialog;
	bool hasRunStart = false, hasRunEnd = false;
};

class Seek : public Function {
public:
	Seek(StateMachine* fsm, GObject* target, bool usePathfinding, SpaceFloat margin = 0.0);
    Seek(StateMachine* fsm, const ValueMap& args);
    
	virtual update_return update();
    
	GetLockmask(movement)
	FuncGetName(Seek)
protected:
	gobject_ref target;
	SpaceFloat margin;
	bool usePathfinding;
};

class ExplodeOnZeroHP : public Function {
public:
	ExplodeOnZeroHP(StateMachine* fsm, DamageInfo damage, SpaceFloat radius);

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();
protected:
	void explode();

	DamageInfo damage;
	SpaceFloat radius;
};

class MaintainDistance : public Function {
public:
    MaintainDistance(StateMachine* fsm, gobject_ref target, SpaceFloat distance, SpaceFloat margin);
    MaintainDistance(StateMachine* fsm, const ValueMap& args);
    
	virtual update_return update();
    GetLockmask(movement)    
    FuncGetName(MaintainDistance)
protected:
	gobject_ref target;
    SpaceFloat distance, margin;
};

class Flock : public Function {
public:
	static const SpaceFloat separationDesired;

	Flock(StateMachine* fsm);

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();

	void onDetectNeighbor(Agent* agent);
	void endDetectNeighbor(Agent* agent);

	SpaceVect separate();
	SpaceVect align();
	SpaceVect cohesion();

	GetLockmask2(movement, look)
	FuncGetName(Flock)
protected:
	unordered_set<object_ref<Agent>> neighbors;
};

class OccupyPoint : public Function {
public:
	OccupyPoint(StateMachine* fsm, SpaceVect target);

	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(OccupyPoint)
protected:
	SpaceVect target;
};

class OccupyMidpoint : public Function {
public:
	OccupyMidpoint(StateMachine* fsm, gobject_ref target1, gobject_ref target2);

	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(OccupyMidpoint)
protected:
	gobject_ref target1, target2;
};

class Scurry : public Function {
public:
	Scurry(
		StateMachine* fsm,
		GObject* _target,
		SpaceFloat distance,
		SpaceFloat length
	);

	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(Scurry)
protected:
	unsigned int startFrame, endFrame;
	SpaceFloat distance;
	bool scurryLeft = true;
	gobject_ref target;
};

class Flee : public Function {
public:
	Flee(StateMachine* fsm, GObject* target, SpaceFloat distance);
    Flee(StateMachine* fsm, const ValueMap& args);
    
	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(Flee)
protected:
	gobject_ref target;
    SpaceFloat distance;
};

class EvadePlayerProjectiles : public Function {
public:
	EvadePlayerProjectiles(StateMachine* fsm);
	EvadePlayerProjectiles(StateMachine* fsm, const ValueMap& args);

	virtual update_return update();

	inline virtual lock_mask getLockMask() {
		return active ? make_enum_bitfield(ResourceLock::movement) : lock_mask();
	}

	FuncGetName(EvadePlayerProjectiles)
protected:
	bool active = false;
};

class IdleWait : public Function{
public:
    IdleWait(StateMachine* fsm, const ValueMap& args);
	IdleWait(StateMachine* fsm, int frames);
	IdleWait(StateMachine* fsm);

	virtual update_return update();
	GetLockmask(movement)
	FuncGetName(IdleWait)
private:
    int remaining;
};

class LookAround : public Function {
public:
	LookAround(StateMachine* fsm, SpaceFloat angularVelocity);

	virtual update_return update();

	GetLockmask(look)
	FuncGetName(LookAround)
private:
	SpaceFloat angularVelocity;
};

class CircleAround : public Function {
public:
	CircleAround(
		StateMachine* fsm,
		SpaceVect center,
		SpaceFloat startingAngularPos,
		SpaceFloat angularSpeed
	);

	virtual void init();
	virtual update_return update();

	GetLockmask(look)
	FuncGetName(CircleAround)
private:
	SpaceVect center;
	SpaceFloat angularSpeed;
	SpaceFloat angularPosition;
};

class Flank : public Function {
public:
	Flank(
		StateMachine* fsm,
		gobject_ref target,
		SpaceFloat desiredDistance,
		SpaceFloat wallMargin
	);

	virtual void init();
	virtual update_return update();
	GetLockmask(movement);
	FuncGetName(Flank);

	bool wallQuery(SpaceVect pos);
private:
	gobject_ref target;
	SpaceFloat desiredDistance;
	SpaceFloat wallMargin;
};

class QuadDirectionLookAround : public Function {
public:
	QuadDirectionLookAround(
		StateMachine* fsm,
		boost::rational<int> secondsPerDirection,
		bool clockwise
	);

	virtual update_return update();

	GetLockmask(look)
	FuncGetName(QuadDirectionLookAround)
private:
	boost::rational<int> secondsPerDirection;
	boost::rational<int> timeRemaining;
	bool clockwise;
};

class AimAtTarget : public Function {
public:
	AimAtTarget(StateMachine* fsm, gobject_ref target);

	virtual update_return update();

	GetLockmask(look)
	FuncGetName(AimAtTarget)
private:
	gobject_ref target;
};

class LookTowardsFire : public Function {
public:
	static constexpr float hitCost = 0.375f;
	static constexpr float lookThresh = 1.0f;
	static constexpr float timeCoeff = 0.5f;
	static constexpr float lookTimeCoeff = 0.125f;

	LookTowardsFire(StateMachine* fsm, bool useShield);

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();

	virtual bool onEvent(Event event);
	virtual event_bitset getEvents();

	virtual lock_mask getLockMask();

	FuncGetName(LookTowardsFire)
protected:
	unsigned int hitCallbackID = 0;
	unsigned int blockCallbackID = 0;
	float hitAccumulator = 0.0f;
	SpaceVect directionAccumulator;
	bool useShield;
	bool looking = false;
};

class MoveToPoint : public Function{
public:
	static const SpaceFloat arrivalMargin;

    MoveToPoint(StateMachine* fsm, const ValueMap& args);
	MoveToPoint(StateMachine* fsm, SpaceVect target);
    
	virtual update_return update();

	GetLockmask(movement)
    FuncGetName(MoveToPoint)
protected:
    SpaceVect target;
};

class BezierMove : public Function {
public:
	BezierMove(StateMachine* fsm, array<SpaceVect, 3> points, SpaceFloat rate);

	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(BezierMove)
protected:
	array<SpaceVect, 3> points;
	SpaceFloat rate;
	SpaceFloat t = 0.0;
};

class PolarMove : public Function {
public:
	PolarMove(StateMachine* fsm, SpaceFloat force, SpaceFloat angularSpeed);

	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(PolarMove)
protected:
	SpaceFloat force;
	SpaceFloat angularSpeed;
};

class FollowPath : public Function {
public:
	static shared_ptr<FollowPath> pathToTarget(
		StateMachine* fsm,
		gobject_ref target
	);

	FollowPath(StateMachine* fsm, const ValueMap& args);
	FollowPath(StateMachine* fsm, Path path, bool loop, bool stopForObstacle);
	inline virtual ~FollowPath() {}

	virtual update_return update();
	GetLockmask(movement)
	FuncGetName(FollowPath)
protected:
	Path path;
	size_t currentTarget = 0;
	bool loop = false;
	bool stopForObstacle = false;
};

class PathToTarget : public FollowPath {
public:
	static shared_ptr<PathToTarget> create(StateMachine* fsm, GObject* target);

	PathToTarget(StateMachine* fsm, Path path, gobject_ref target);

	virtual update_return update();
	FuncGetName(PathToTarget)
protected:
	gobject_ref target;
};

class Wander : public Function {
public:
    Wander(StateMachine* fsm, const ValueMap& args);
	Wander(StateMachine* fsm, SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist);
	Wander(StateMachine* fsm, SpaceFloat waitInterval, SpaceFloat moveDist);
	Wander(StateMachine* fsm);

	pair<Direction, SpaceFloat> chooseMovement();

	virtual update_return update();

	GetLockmask(movement)
	FuncGetName(Wander)
protected:
    SpaceFloat minWait, maxWait;
    SpaceFloat minDist, maxDist;
	SpaceFloat waitTimer = 0.0;
};

class Operation : public Function {
public:
	Operation(StateMachine* fsm, std::function<void(StateMachine&)> op);
	
	virtual update_return update();
	FuncGetName(Operation)
protected:
    std::function<void(StateMachine&)> op;
};

class Cast : public Function {
public:
	Cast(StateMachine* fsm, SpellGeneratorType spell_generator, SpaceFloat length = 0.0);

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();

	GetLockmask(spellcasting)
	FuncGetName(Cast)
protected:
	SpellGeneratorType spell_generator;
	SpaceFloat timer = 0.0;
	SpaceFloat length;
};

class HPCast : public Function {
public:
	HPCast(StateMachine* fsm, SpellGeneratorType spell_generator, float hp_difference);

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();

	GetLockmask(spellcasting)
	FuncGetName(HPCast)
protected:
	SpellGeneratorType spell_generator;
	float caster_starting;
	float hp_difference;
};

class HPCastSequence : public Function {
public:
	HPCastSequence(
		StateMachine* fsm,
		const vector<SpellGeneratorType>& spells,
		const boost::icl::interval_map<float, int> intervals
	);

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();

	GetLockmask(spellcasting)
	FuncGetName(HPCastSequence)
protected:
	int crntInterval = -1;
	vector<SpellGeneratorType> spells;
	boost::icl::interval_map<float, int> intervals;
};

class FireOnStress : public Function {
public:
	FireOnStress(StateMachine* fsm, float stressPerShot);

	virtual update_return update();

	GetLockmask(fire)
	FuncGetName(FireOnStress)
protected:
	float stressPerShot;
};

class FireAtTarget : public Function {
public:
	FireAtTarget(StateMachine* fsm, gobject_ref target);

	virtual update_return update();

	GetLockmask2(look,fire)
	FuncGetName(FireAtTarget)
protected:
	gobject_ref target;
};

class FireIfTargetVisible : public Function {
public:
	FireIfTargetVisible(StateMachine* fsm, gobject_ref target);

	virtual update_return update();

	GetLockmask(fire)
	FuncGetName(FireIfTargetsVisible)
protected:
	gobject_ref target;
};

class ThrowBombs : public Function {
public:
	ThrowBombs(
		StateMachine* fsm,
		gobject_ref target,
		string bombType,
		SpaceFloat throwingSpeed,
		SpaceFloat baseInterval,
		SpaceFloat blastRadius,
		SpaceFloat fuseTime,
		float cost
	);

	virtual void init();
	virtual update_return update();

	SpaceFloat getInterval();
	float score(SpaceVect pos, SpaceFloat angle);

	GetLockmask(bomb)
	FuncGetName(ThrowBombs)
protected:
	SpaceFloat countdown;
	SpaceFloat throwingSpeed;
	SpaceFloat baseInterval;
	SpaceFloat blastRadius;
	SpaceFloat fuseTime;
	gobject_ref target;
	string bombType;
	float cost;
};


} //end NS

#endif /* AIFunctions_hpp */
