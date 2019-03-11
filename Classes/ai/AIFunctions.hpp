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

class Seek : public Function {
public:
	Seek(GObject* target, bool usePathfinding, SpaceFloat margin = 0.0);
    Seek(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);
    
	GetLockmask(movement)
	FuncGetName(Seek)
protected:
	gobject_ref target;
	SpaceFloat margin;
	bool usePathfinding;
};

class MaintainDistance : public Function {
public:
    MaintainDistance(gobject_ref target, SpaceFloat distance, SpaceFloat margin);
    MaintainDistance(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);
    
    GetLockmask(movement)    
    FuncGetName(MaintainDistance)
protected:
	gobject_ref target;
    SpaceFloat distance, margin;
};

class Flock : public Function {
public:
	static const SpaceFloat separationDesired;

	Flock();

	virtual void update(StateMachine& sm);

	void onDetectNeighbor(Agent* agent);
	void endDetectNeighbor(Agent* agent);

	SpaceVect separate(Agent* _agent);
	SpaceVect align(Agent* _agent);
	SpaceVect cohesion(Agent* _agent);

	GetLockmask2(movement, look)
	FuncGetName(Flock)
protected:
	unordered_set<object_ref<Agent>> neighbors;
};

class OccupyPoint : public Function {
public:
	OccupyPoint(SpaceVect target);

	virtual void update(StateMachine& sm);

	GetLockmask(movement)
	FuncGetName(OccupyPoint)
protected:
	SpaceVect target;
};

class OccupyMidpoint : public Function {
public:
	OccupyMidpoint(gobject_ref target1, gobject_ref target2);

	virtual void update(StateMachine& sm);

	GetLockmask(movement)
	FuncGetName(OccupyMidpoint)
protected:
	gobject_ref target1, target2;
};

class Scurry : public Function {
public:
	Scurry(GSpace* space, GObject* _target, SpaceFloat distance, SpaceFloat length);

	virtual void update(StateMachine& sm);

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
	Flee(GObject* target, SpaceFloat distance);
    Flee(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);

	GetLockmask(movement)
	FuncGetName(Flee)
protected:
	gobject_ref target;
    SpaceFloat distance;
};

class EvadePlayerProjectiles : public Function {
public:
	EvadePlayerProjectiles();
	EvadePlayerProjectiles(GSpace* space, const ValueMap& args);

	virtual void update(StateMachine& sm);

	inline virtual bitset<lockCount> getLockMask() {
		return active ? make_enum_bitfield(ResourceLock::movement) : bitset<lockCount>();
	}

	FuncGetName(EvadePlayerProjectiles)
protected:
	bool active = false;
};

class IdleWait : public Function{
public:
    IdleWait(GSpace* space, const ValueMap& args);
	IdleWait(int frames);
	IdleWait();

	virtual void update(StateMachine& fsm);
    
	GetLockmask(movement)
	FuncGetName(IdleWait)
private:
    int remaining;
};

class LookAround : public Function {
public:
	LookAround(SpaceFloat angularVelocity);

	virtual void update(StateMachine& fsm);

	GetLockmask(look)
	FuncGetName(LookAround)
private:
	SpaceFloat angularVelocity;
};

class CircleAround : public Function {
public:
	CircleAround(SpaceVect center, SpaceFloat startingAngularPos, SpaceFloat angularSpeed);

	virtual void init(StateMachine& fsm);
	virtual void update(StateMachine& fsm);

	GetLockmask(look)
	FuncGetName(CircleAround)
private:
	SpaceVect center;
	SpaceFloat angularSpeed;
	SpaceFloat angularPosition;
};

class Flank : public Function {
public:
	Flank(gobject_ref target, SpaceFloat desiredDistance, SpaceFloat wallMargin);

	virtual void init(StateMachine& fsm);
	virtual void update(StateMachine& fsm);
	GetLockmask(movement);
	FuncGetName(Flank);

	bool wallQuery(StateMachine& fsm,SpaceVect pos);
private:
	gobject_ref target;
	SpaceFloat desiredDistance;
	SpaceFloat wallMargin;
};

class QuadDirectionLookAround : public Function {
public:
	QuadDirectionLookAround(boost::rational<int> secondsPerDirection, bool clockwise);

	virtual void update(StateMachine& fsm);

	GetLockmask(look)
	FuncGetName(QuadDirectionLookAround)
private:
	boost::rational<int> secondsPerDirection;
	boost::rational<int> timeRemaining;
	bool clockwise;
};

class AimAtTarget : public Function {
public:
	AimAtTarget(gobject_ref target);

	virtual void update(StateMachine& fsm);

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

	LookTowardsFire();

	virtual void onEnter(StateMachine& fsm);
	virtual void update(StateMachine& fsm);
	virtual void onExit(StateMachine& fsm);

	void onBulletCollide(StateMachine& fsm, Bullet* b);

	virtual bitset<lockCount> getLockMask();

	FuncGetName(LookTowardsFire)
protected:
	float hitAccumulator = 0.0f;
	SpaceVect directionAccumulator;
	bool looking = false;
};

class MoveToPoint : public Function{
public:
	static const SpaceFloat arrivalMargin;

    MoveToPoint(GSpace* space, const ValueMap& args);
	MoveToPoint(SpaceVect target);
    
    virtual void update(StateMachine& fsm);

	GetLockmask(movement)
    FuncGetName(MoveToPoint)
protected:
    SpaceVect target;
};

class BezierMove : public Function {
public:
	BezierMove(array<SpaceVect, 3> points, SpaceFloat rate);

	virtual void update(StateMachine& fsm);

	GetLockmask(movement)
	FuncGetName(BezierMove)
protected:
	array<SpaceVect, 3> points;
	SpaceFloat rate;
	SpaceFloat t = 0.0;
};

class FollowPath : public Function {
public:
	static shared_ptr<FollowPath> pathToTarget(GSpace* space, gobject_ref agent, gobject_ref target);

	FollowPath(GSpace* space, const ValueMap& args);
	FollowPath(Path path, bool loop, bool stopForObstacle);

	virtual void update(StateMachine& fsm);
    
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
	static shared_ptr<PathToTarget> create(GObject* agent, GObject* target);

	PathToTarget(Path path, gobject_ref target);

	virtual void update(StateMachine& sm);
	FuncGetName(PathToTarget)
protected:
	gobject_ref target;
};

class Wander : public Function {
public:
    Wander(GSpace* space, const ValueMap& args);
	Wander(SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist);
	Wander(SpaceFloat waitInterval, SpaceFloat moveDist);
	Wander();

	pair<Direction, SpaceFloat> chooseMovement(StateMachine& fsm);

    virtual void update(StateMachine& sm);

	GetLockmask(movement)
	FuncGetName(Wander)
protected:
    SpaceFloat minWait, maxWait;
    SpaceFloat minDist, maxDist;
	SpaceFloat waitTimer = 0.0;
};

class Operation : public Function {
public:
	Operation(std::function<void(StateMachine&)> op);
	
	virtual void update(StateMachine& sm);
    
	FuncGetName(Operation)
protected:
    std::function<void(StateMachine&)> op;
};

class Cast : public Function {
public:
	Cast(SpellGeneratorType spell_generator);

	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
	virtual void onExit(StateMachine& sm);

	GetLockmask(spellcasting)
	FuncGetName(Cast)
protected:
	SpellGeneratorType spell_generator;
};

class HPCast : public Function {
public:
	HPCast(SpellGeneratorType spell_generator, float hp_difference);

	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
	virtual void onExit(StateMachine& sm);

	GetLockmask(spellcasting)
	FuncGetName(HPCast)
protected:
	SpellGeneratorType spell_generator;
	float caster_starting;
	float hp_difference;
};

class HPCastSequence : public Function {
public:
	HPCastSequence(const vector<SpellGeneratorType>& spells, const boost::icl::interval_map<float, int> intervals);

	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
	virtual void onExit(StateMachine& sm);

	GetLockmask(spellcasting)
	FuncGetName(HPCastSequence)
protected:
	int crntInterval = -1;
	vector<SpellGeneratorType> spells;
	boost::icl::interval_map<float, int> intervals;
};

class FireOnStress : public Function {
public:
	FireOnStress(float stressPerShot);

	virtual void update(StateMachine& sm);

	GetLockmask(fire)
	FuncGetName(FireOnStress)
protected:
	float stressPerShot;
};

class FireAtTarget : public Function {
public:
	FireAtTarget(gobject_ref target);

	virtual void update(StateMachine& sm);

	GetLockmask2(look,fire)
	FuncGetName(FireAtTarget)
protected:
	gobject_ref target;
};

class FireIfTargetVisible : public Function {
public:
	FireIfTargetVisible(gobject_ref target);

	virtual void update(StateMachine& sm);

	GetLockmask(fire)
	FuncGetName(FireIfTargetsVisible)
protected:
	gobject_ref target;
};

} //end NS

#endif /* AIFunctions_hpp */
