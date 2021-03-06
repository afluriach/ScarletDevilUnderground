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

class MoveToPoint;

class OnDetect : public Function {
public:
	OnDetect(StateMachine* fsm, GType type, AITargetFunctionGenerator gen);

	virtual void detect(GObject* obj);
	virtual event_type getEvents();
protected:
	local_shared_ptr<Thread> thread;
	AITargetFunctionGenerator gen;
	GType type;
};

class WhileDetect : public Function {
public:
	WhileDetect(StateMachine* fsm, GType type, AITargetFunctionGenerator gen);

	virtual void detect(GObject* obj);
	virtual void endDetect(GObject* obj);
	virtual event_type getEvents();
protected:
	local_shared_ptr<Thread> thread;
	AITargetFunctionGenerator gen;
	GType type;
};

class OnAlert : public Function {
public:
	OnAlert(StateMachine* fsm, AITargetFunctionGenerator gen);

	virtual void roomAlert(Player* p);
	virtual event_type getEvents();
protected:
	AITargetFunctionGenerator gen;
};

class OnAlertFunction : public Function {
public:
	OnAlertFunction(StateMachine* fsm, alert_function f);

	virtual void roomAlert(Player* p);
	virtual event_type getEvents();
protected:
	alert_function f;
};

class CompositeFunction : public Function {
public:
	//create empty
	CompositeFunction(StateMachine* fsm);

	template<class FuncCls, typename... Params>
	inline void addFunction(Params... params)
	{
		addFunction(make_local_shared<FuncCls>(fsm, params...));
	}

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();
	virtual string getName();

	void addFunction(local_shared_ptr<Function> f);
	void removeFunction(local_shared_ptr<Function> f);
protected:
	list<local_shared_ptr<Function>> functions;
	bool hasInit = false;
};

class ScriptFunction : public Function {
public:
	static AITargetFunctionGenerator targetGenerator(const string& cls);

	ScriptFunction(StateMachine* fsm, const string& cls);
	ScriptFunction(StateMachine* fsm, GObject* target, const string& cls);

	virtual void onEnter();
	virtual update_return update();
	virtual void onExit();

	virtual void bulletBlock(Bullet* b);
	virtual void bulletHit(Bullet* b);

	virtual void detect(GObject* _obj);
	virtual void endDetect(GObject* _obj);

	virtual void roomAlert(Player* p);

	virtual void zeroHP();
	virtual void zeroStamina();

	event_type getEvents();
	 virtual string getName();
protected:
	event_type checkEventMethods();
	bool hasMethod(const string& name);

	string cls;
	sol::table obj;
};

class BossFightHandler : public Function {
public:
	BossFightHandler(StateMachine* fsm, string startDialog, string endDialog);

	virtual void detect(GObject* obj);
	virtual void zeroHP();

	virtual event_type getEvents();
	FuncGetName(BossFightHandler);
protected:
	string startDialog, endDialog;
	bool hasRunStart = false, hasRunEnd = false;
};

class Seek : public Function {
public:
	static constexpr double pathfindingCooldown = 0.25;

	enum class states {
		direct_seek,
		pathfinding,
		arriving,
		no_target,
	};

	Seek(StateMachine* fsm, GObject* target, bool usePathfinding, SpaceFloat margin = 0.0);
    
	virtual update_return update();
    
	FuncGetName(Seek)
protected:
	local_shared_ptr<Function> pathFunction;
	gobject_ref target;
	SpaceFloat margin;
	SpaceFloat lastPathfind = 0.0;
	states crntState = states::direct_seek;
	bool usePathfinding;
};

class ExplodeOnZeroHP : public Function {
public:
	ExplodeOnZeroHP(StateMachine* fsm, DamageInfo damage, SpaceFloat radius);

	virtual void zeroHP();
	virtual event_type getEvents();
protected:
	void explode();

	DamageInfo damage;
	SpaceFloat radius;
};

class MaintainDistance : public Function {
public:
    MaintainDistance(StateMachine* fsm, gobject_ref target, SpaceFloat distance, SpaceFloat margin);
    
	virtual update_return update();
    FuncGetName(MaintainDistance)
protected:
	gobject_ref target;
    SpaceFloat distance, margin;
};

class OccupyPoint : public Function {
public:
	OccupyPoint(StateMachine* fsm, SpaceVect target);

	virtual update_return update();

	FuncGetName(OccupyPoint)
protected:
	SpaceVect target;
};

class OccupyMidpoint : public Function {
public:
	OccupyMidpoint(StateMachine* fsm, gobject_ref target1, gobject_ref target2);

	virtual update_return update();

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

	FuncGetName(Scurry)
protected:
	local_shared_ptr<MoveToPoint> moveFunction;
	unsigned int startFrame, endFrame;
	SpaceFloat distance;
	bool scurryLeft = true;
	gobject_ref target;
};

class Flee : public Function {
public:
	Flee(StateMachine* fsm, GObject* target, SpaceFloat distance);
    
	virtual update_return update();

	FuncGetName(Flee)
protected:
	gobject_ref target;
    SpaceFloat distance;
};

class Evade : public Function {
public:
	Evade(StateMachine* fsm, GType type);

	virtual update_return update();
	inline virtual bool isActive() { return active; }

	FuncGetName(Evade)
protected:
	GType type;
	bool active = false;
};

class LookAround : public Function {
public:
	LookAround(StateMachine* fsm, SpaceFloat angularVelocity);

	virtual update_return update();

	FuncGetName(LookAround)
private:
	SpaceFloat angularVelocity;
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
	FuncGetName(Flank);

	bool wallQuery(SpaceVect pos);
private:
	local_shared_ptr<MoveToPoint> moveFunction;
	gobject_ref target;
	SpaceFloat desiredDistance;
	SpaceFloat wallMargin;
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

	virtual void bulletHit(Bullet* b);
	virtual event_type getEvents();

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
    
	virtual inline bool isCompleted() const { return arrived; }
	virtual update_return update();

    FuncGetName(MoveToPoint)
protected:
    SpaceVect target;
	bool arrived = false;
};

class FollowPath : public Function {
public:
	static local_shared_ptr<FollowPath> pathToTarget(
		StateMachine* fsm,
		gobject_ref target
	);

	FollowPath(StateMachine* fsm, const ValueMap& args);
	FollowPath(StateMachine* fsm, Path path, bool loop, bool stopForObstacle);
	inline virtual ~FollowPath() {}

	virtual inline bool isCompleted() const { return completed; }
	virtual update_return update();
	FuncGetName(FollowPath)
protected:
	Path path;
	size_t currentTarget = 0;
	bool loop = false;
	bool stopForObstacle = false;
	bool completed = false;
};

class Wander : public Function {
public:
    Wander(StateMachine* fsm, const ValueMap& args);
	Wander(StateMachine* fsm, SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist);
	Wander(StateMachine* fsm, SpaceFloat waitInterval, SpaceFloat moveDist);
	Wander(StateMachine* fsm);

	pair<Direction, SpaceFloat> chooseMovement();

	virtual update_return update();
	virtual void reset();

	FuncGetName(Wander)
protected:
	local_shared_ptr<MoveToPoint> moveFunction;
    SpaceFloat minWait, maxWait;
    SpaceFloat minDist, maxDist;
	SpaceFloat waitTimer = 0.0;
};

class FireOnStress : public Function {
public:
	FireOnStress(StateMachine* fsm, float stressPerShot);

	virtual update_return update();

	FuncGetName(FireOnStress)
protected:
	float stressPerShot;
};

class FireAtTarget : public Function {
public:
	FireAtTarget(StateMachine* fsm, gobject_ref target);

	virtual update_return update();

	FuncGetName(FireAtTarget)
protected:
	gobject_ref target;
};

class ThrowBombs : public Function {
public:
	ThrowBombs(
		StateMachine* fsm,
		gobject_ref target,
		local_shared_ptr<bomb_properties> bombType,
		SpaceFloat throwingSpeed,
		SpaceFloat baseInterval
	);

	virtual void init();
	virtual update_return update();

	SpaceFloat getInterval();
	float score(SpaceVect pos, SpaceFloat angle);

	FuncGetName(ThrowBombs)
protected:
	local_shared_ptr<bomb_properties> bombType;

	SpaceFloat countdown;
	SpaceFloat throwingSpeed;
	SpaceFloat baseInterval;
	gobject_ref target;
};


} //end NS

#endif /* AIFunctions_hpp */
