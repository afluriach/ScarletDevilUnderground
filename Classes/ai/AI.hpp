//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

#include "enum.h"
#include "object_ref.hpp"
#include "types.h"

class Agent;
class GObject;
class GSpace;

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject* obj, SpaceVect desired, SpaceFloat maxForce);
SpaceVect compute_seek(Agent* agent, SpaceVect target);
void seek(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
void arrive(GObject* agent, SpaceVect target);
SpaceVect fleeDirection(const GObject* agent, SpaceVect target);
void flee(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
void fleeWithObstacleAvoidance(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
bool moveToPoint(GObject* agent, SpaceVect target, SpaceFloat arrivalMargin, bool stopForObstacle);

bool isFacingTarget(const GObject* agent, const GObject* target);
bool isFacingTargetsBack(const GObject* agent, const GObject* target);
bool isLineOfSight(const GObject* agent, const GObject* target);
bool isObstacle(Agent* agent, SpaceVect target);

array<SpaceFloat, 8> wallFeeler8(const GObject* agent, SpaceFloat distance);
array<SpaceFloat, 4> obstacleFeelerQuad(const GObject* agent, SpaceFloat distance);
array<SpaceFloat, 8> obstacleFeeler8(const GObject* agent, SpaceFloat distance);
int chooseBestDirection(const array<SpaceFloat, 8>& feelers, SpaceFloat desired_angle, SpaceFloat min_distance);

SpaceVect directionToTarget(const GObject* agent, SpaceVect target);
SpaceVect displacementToTarget(const GObject* agent, SpaceVect target);
SpaceFloat distanceToTarget(const GObject* agent, const GObject* target);
SpaceFloat distanceToTarget(const GObject* agent, SpaceVect target);
SpaceFloat viewAngleToTarget(const GObject* agent, const GObject* target);
SpaceVect projectileEvasion(const GObject* bullet, const GObject* agent);
bool isInFieldOfView(GObject* agent, SpaceVect target, SpaceFloat fovAngleScalarProduct);

SpaceFloat getStoppingTime(SpaceFloat speed, SpaceFloat acceleration);
SpaceFloat getStoppingDistance(SpaceFloat speed, SpaceFloat accceleration);
SpaceFloat getTurningRadius(SpaceFloat speed, SpaceFloat acceleration);

SpaceVect bezier(array<SpaceVect, 3> points, SpaceFloat t);
SpaceVect bezierAcceleration(array<SpaceVect, 3> points);

bullet_collide_function buildStressFromHits(float hpStressScale);

enum class ResourceLock
{
    begin = 0,
    movement = 0,
	look,
	spellcasting,
	fire,
    
    end,
};

constexpr size_t lockCount = to_size_t(ResourceLock::end);

class Thread;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}

class Function
{
public:
	friend class Thread;

	inline virtual ~Function() {}

    typedef function<shared_ptr<Function>(GSpace* space, const ValueMap&) > AdapterType;
    static const unordered_map<string, Function::AdapterType> adapters;
    
    static shared_ptr<Function> constructState(const string& type, GSpace* space, const ValueMap& args);

	inline virtual void onEnter(StateMachine& sm) {}
    inline virtual void onReturn(StateMachine& sm) {}
	inline virtual void update(StateMachine& sm) {}
	inline virtual void onExit(StateMachine& sm) {}
    
    inline virtual void onDelay(StateMachine& sm) {}
    
    inline virtual string getName() const {return "Function";}
    
    inline virtual bitset<lockCount> getLockMask() { return bitset<lockCount>();}
protected:
	bool hasRunInit = false;
};

class Thread
{
public:
    typedef int priority_type;
    
	friend class StateMachine;
    
    const unsigned int uuid;

	Thread(shared_ptr<Function> threadMain, StateMachine* sm);
	Thread(
        shared_ptr<Function> threadMain,
        StateMachine* sm,
        priority_type priority,
        bitset<lockCount> lockMask
    );

	void update(StateMachine& sm);
    
    void onDelay(StateMachine& sm);

	void push(shared_ptr<Function> newState);
	void pop();
    
    string getStack();
    
	string getMainFuncName();
	void setResetOnBlock(bool reset);

protected:
	list<shared_ptr<Function>> call_stack;
	StateMachine* sm;
	bool completed = false;
    bool resetOnBlock = false;
    priority_type priority = 0;
    bitset<lockCount> lockMask;
    static unsigned int nextUUID;
};

class StateMachine
{
public:
    GObject *const agent;

    StateMachine(GObject *const agent);

	void update();

	void addThread(shared_ptr<Thread> thread);
    unsigned int addThread(shared_ptr<Function> threadMain, Thread::priority_type priority = 0);
    void removeThread(unsigned int uuid);
    //Remove thread(s) that have the given main function.
    void removeThread(const string& mainName);
	void removeCompletedThreads();
	bool isThreadRunning(const string& mainName);
	void applyAddThreads();
	void applyRemoveThreads();

    void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	void onBulletHit(Bullet* b);
	void onAlert(Player* p);

	void addDetectFunction(GType t, detect_function f);
	void addEndDetectFunction(GType t, detect_function f);
	void removeDetectFunction(GType t);
	void removeEndDetectFunction(GType t);
	void setBulletHitFunction(bullet_collide_function f);
	void setAlertFunction(alert_function f);

	//wrappers for the current thread
	void push(shared_ptr<Function> f);
	void pop();
    
	Agent* getAgent();
	unsigned int getFrame();
	Thread* getCrntThread();
    string toString();
protected:
	unordered_set<unsigned int> threadsToRemove;
	list<shared_ptr<Thread>> threadsToAdd;

	bullet_collide_function bulletHandler;
	alert_function alertHandler;
	unordered_map<GType, detect_function> detectHandlers;
	unordered_map<GType, detect_function> endDetectHandlers;

	map<unsigned int,shared_ptr<Thread>> current_threads;
    map<int, list<unsigned int>> threads_by_priority;
    unsigned int frame;
	Thread* crntThread = nullptr;
	bool alerted = false;
};

class Seek : public Function {
public:
	Seek(GObject* target, bool usePathfinding);
    Seek(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(Seek)
protected:
	gobject_ref target;
	bool usePathfinding;
};

class MaintainDistance : public Function {
public:
    MaintainDistance(gobject_ref target, SpaceFloat distance, SpaceFloat margin);
    MaintainDistance(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    
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

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement) | make_enum_bitfield(ResourceLock::look);
	}

	void onDetectNeighbor(Agent* agent);
	void endDetectNeighbor(Agent* agent);

	SpaceVect separate(Agent* _agent);
	SpaceVect align(Agent* _agent);
	SpaceVect cohesion(Agent* _agent);

	FuncGetName(Flock)
protected:
	unordered_set<object_ref<Agent>> neighbors;
};

class OccupyPoint : public Function {
public:
	OccupyPoint(SpaceVect target);

	virtual void update(StateMachine& sm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}

	FuncGetName(OccupyPoint)
protected:
	SpaceVect target;
};

class OccupyMidpoint : public Function {
public:
	OccupyMidpoint(gobject_ref target1, gobject_ref target2);

	virtual void update(StateMachine& sm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}

	FuncGetName(OccupyMidpoint)
protected:
	gobject_ref target1, target2;
};

class Scurry : public Function {
public:
	Scurry(GSpace* space, GObject* _target, SpaceFloat distance, SpaceFloat length);

	virtual void update(StateMachine& sm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}

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

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    
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
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    
    FuncGetName(IdleWait)
private:
    int remaining;
};

class LookAround : public Function {
public:
	LookAround(SpaceFloat angularVelocity);

	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::look);
	}

	FuncGetName(LookAround)
private:
	SpaceFloat angularVelocity;
};

class CircleAround : public Function {
public:
	CircleAround(SpaceVect center, SpaceFloat startingAngularPos, SpaceFloat angularSpeed);

	virtual void init(StateMachine& fsm);
	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::look);
	}

	FuncGetName(CircleAround)
private:
	SpaceVect center;
	SpaceFloat angularSpeed;
	SpaceFloat angularPosition;
};

class Flank : public Function {
public:
	Flank(gobject_ref target, SpaceFloat desiredDistance);

	virtual void init(StateMachine& fsm);
	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}

	FuncGetName(Flank)
private:
	gobject_ref target;
	SpaceFloat desiredDistance;
};

class QuadDirectionLookAround : public Function {
public:
	QuadDirectionLookAround(boost::rational<int> secondsPerDirection, bool clockwise);

	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::look);
	}

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

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::look);
	}

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

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(MoveToPoint)
protected:
    SpaceVect target;
};

class BezierMove : public Function {
public:
	BezierMove(array<SpaceVect, 3> points, SpaceFloat rate);

	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}
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
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement) | make_enum_bitfield(ResourceLock::look);
    }
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

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
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

	FuncGetName(Cast)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::spellcasting);
	}

protected:
	SpellGeneratorType spell_generator;
};

class HPCast : public Function {
public:
	HPCast(SpellGeneratorType spell_generator, float hp_difference);

	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
	virtual void onExit(StateMachine& sm);

	FuncGetName(HPCast)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::spellcasting);
	}

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

	FuncGetName(HPCastSequence)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::spellcasting);
	}

protected:
	int crntInterval = -1;
	vector<SpellGeneratorType> spells;
	boost::icl::interval_map<float, int> intervals;
};

class BuildStressFromPlayerProjectiles: public Function
{
public:
	BuildStressFromPlayerProjectiles(float scale);

	virtual void onEnter(StateMachine& sm);
	virtual void onExit(StateMachine& sm);

	FuncGetName(BuildStressFromPlayerProjectiles)

	inline virtual bitset<lockCount> getLockMask() { return bitset<lockCount>(); }
protected:
	float scale;
};

class FireOnStress : public Function {
public:
	FireOnStress(float stressPerShot);

	virtual void update(StateMachine& sm);
	FuncGetName(FireOnStress)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::fire);
	}
protected:
	float stressPerShot;
};

class FireAtTarget : public Function {
public:
	FireAtTarget(gobject_ref target);

	virtual void update(StateMachine& sm);

	FuncGetName(FireAtTarget)
	
	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::look) | make_enum_bitfield(ResourceLock::fire);
	}

protected:
	gobject_ref target;
};

class FireIfTargetVisible : public Function {
public:
	FireIfTargetVisible(gobject_ref target);

	virtual void update(StateMachine& sm);

	FuncGetName(FireIfTargetsVisible)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::fire);
	}

protected:
	gobject_ref target;
};

} //end NS

#endif /* AI_hpp */
