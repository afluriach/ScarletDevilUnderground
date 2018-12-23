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
#include "GSpace.hpp"
#include "object_ref.hpp"
#include "types.h"

class Agent;
class FirePattern;
class GObject;
class GSpace;

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject* obj, SpaceVect desired, SpaceFloat maxForce);
void seek(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
SpaceVect fleeDirection(const GObject* agent, SpaceVect target);
void flee(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
void fleeWithObstacleAvoidance(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);

bool isFacingTarget(const GObject* agent, const GObject* target);
bool isFacingTargetsBack(const GObject* agent, const GObject* target);
bool isLineOfSight(const GObject* agent, const GObject* target);

array<SpaceFloat, 4> obstacleFeelerQuad(const GObject* agent, SpaceFloat distance);
array<SpaceFloat, 8> obstacleFeeler8(const GObject* agent, SpaceFloat distance);
int chooseBestDirection(const array<SpaceFloat, 8>& feelers, SpaceFloat desired_angle, SpaceFloat min_distance);

SpaceVect directionToTarget(const GObject* agent, SpaceVect target);
SpaceVect displacementToTarget(const GObject* agent, SpaceVect target);
SpaceFloat distanceToTarget(const GObject* agent, const GObject* target);
SpaceFloat viewAngleToTarget(const GObject* agent, const GObject* target);
SpaceVect projectileEvasion(const GObject* bullet, const GObject* agent);

SpaceFloat getStoppingTime(SpaceFloat speed, SpaceFloat acceleration);
SpaceFloat getStoppingDistance(SpaceFloat speed, SpaceFloat accceleration);
SpaceFloat getTurningRadius(SpaceFloat speed, SpaceFloat acceleration);

enum class ResourceLock
{
    begin = 0,
    movement = 0,
	spellcasting,
	fire,
    
    end,
};

constexpr size_t lockCount = to_size_t(ResourceLock::end);

class StateMachine;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}

class Function
{
public:

	inline virtual ~Function() {}

    typedef function<shared_ptr<Function>(GSpace* space, const ValueMap&) > AdapterType;
    static const unordered_map<string, Function::AdapterType> adapters;
    
    static shared_ptr<Function> constructState(const string& type, GSpace* space, const ValueMap& args);

	inline virtual void onEnter(StateMachine& sm) {}
    inline virtual void onReturn(StateMachine& sm) {}
	inline virtual void update(StateMachine& sm) {}
	inline virtual void onExit(StateMachine& sm) {}
    
    inline virtual void onDelay(StateMachine& sm) {}

	inline virtual void onDetect(StateMachine& sm, GObject* obj) {}
	inline virtual void onEndDetect(StateMachine& sm, GObject* obj) {}
    
    inline virtual string getName() const {return "Function";}
    
    inline virtual bitset<lockCount> getLockMask() { return bitset<lockCount>();}
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

	void onDetect(StateMachine& sm, GObject* obj);
	void onEndDetect(StateMachine& sm, GObject* obj);
    
    string getStack();
    
    inline string getMainFuncName(){
        return call_stack.front()->getName();
    }
    
    inline void setResetOnBlock(bool reset){
        resetOnBlock = reset;
    }

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
    unsigned int addThread(shared_ptr<Function> threadMain);
    void removeThread(unsigned int uuid);
    //Remove thread(s) that have the given main function.
    void removeThread(const string& mainName);
	void removeCompletedThreads();
	void applyAddThreads();
	void applyRemoveThreads();

    void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	//wrappers for the current thread
	void push(shared_ptr<Function> f);
	void pop();
    
	Agent* getAgent();

    inline unsigned int getFrame(){
        return frame;
    }

	inline Thread* getCrntThread() {
		return crntThread;
	}
    
    string toString();
    
protected:
	set<unsigned int> threadsToRemove;
	list<shared_ptr<Thread>> threadsToAdd;

	map<unsigned int,shared_ptr<Thread>> current_threads;
    map<int, list<unsigned int>> threads_by_priority;
    unsigned int frame;
	Thread* crntThread = nullptr;
};

class Detect : public Function{
public:
    typedef function<shared_ptr<Function>(GObject* detected)> Generator;

    Detect(const string& target_name, Generator nextState);
    Detect(GSpace* space, const ValueMap& args);

    virtual void onDetect(StateMachine& sm, GObject* obj);
    
    FuncGetName(Detect)
protected:
    string target_name;
    Generator nextState;
};

template<typename T>
class TrackByType : public Function {
public:
	//typedef function<shared_ptr<Function>(GObject* detected)> Generator;

	inline TrackByType() {}
	inline TrackByType(GSpace* space, const ValueMap& args) {}

	inline virtual void onEnter(StateMachine& sm) {
		targets = sm.agent->space->getObjectsByTypeAs<T>();

		for (auto it = targets.begin(); it != targets.end(); ) {
			if (it->get() == sm.agent)
				it = targets.erase(it);
			else
				++it;
		}

	}
	inline virtual void update(StateMachine& sm) {
		for (auto it = targets.begin(); it != targets.end(); ) {
			if (!it->isValid())
				it = targets.erase(it);
			else
				++it;
		}
	}

	inline virtual string getName() const {
		return
			string("TrackByType<")  + 
			string(type_index(typeid(T)).name()) +
			string(">")
		;
	}

	template<typename...Args>
	inline void messageTargets(void (T::*m)(Args...), Args ...args)
	{
		for (object_ref<T> _ref : targets) {
			T* t = _ref.get();
			if (t) {
				t->message(t,m,args...);
			}
		}
	}


protected:
	vector<object_ref<T>> targets;
//	Generator nextState;
};


class Seek : public Function {
public:
    inline Seek(GObject* target) : target(target)
    {}
    Seek(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);
	virtual void onEndDetect(StateMachine& sm, GObject* target);
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(Seek)
protected:
	gobject_ref target;
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


class Flee : public Function {
public:
    inline Flee(GObject* target, SpaceFloat distance) :
    target(target),
    distance(distance)
    {}
    Flee(GSpace* space, const ValueMap& args);
    
	virtual void update(StateMachine& sm);
	virtual void onEndDetect(StateMachine& sm, GObject* target);

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
		return make_enum_bitfield(ResourceLock::movement);
	}

	FuncGetName(EvadePlayerProjectiles)
protected:
	list<unsigned int> bullets;
};

class DetectAndSeekPlayer : public Detect{
public:
    DetectAndSeekPlayer();
    DetectAndSeekPlayer(GSpace* space, const ValueMap& args);
    
    FuncGetName(DetectAndSeekPlayer)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}

};

class IdleWait : public Function{
    public:
        IdleWait(GSpace* space, const ValueMap& args);

        inline IdleWait(unsigned int frames) :
        remaining(frames)
        {}
    
		virtual void update(StateMachine& fsm);
    
        inline virtual bitset<lockCount> getLockMask() {
            return make_enum_bitfield(ResourceLock::movement);
        }
    
        FuncGetName(IdleWait)
    private:
        unsigned int remaining;
};

class LookAround : public Function {
public:
	LookAround(SpaceFloat angularVelocity);

	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
	}

	FuncGetName(LookAround)
private:
	SpaceFloat angularVelocity;
};

class QuadDirectionLookAround : public Function {
public:
	QuadDirectionLookAround(boost::rational<int> secondsPerDirection, bool clockwise);

	virtual void update(StateMachine& fsm);

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::movement);
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
		return make_enum_bitfield(ResourceLock::movement);
	}

	FuncGetName(AimAtTarget)
private:
	gobject_ref target;
};


class MoveToPoint : public Function{
public:
    MoveToPoint(GSpace* space, const ValueMap& args);

    inline MoveToPoint(SpaceVect target) :
    target(target)
    {}
    
    virtual void update(StateMachine& fsm);

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(MoveToPoint)
protected:
    SpaceVect target;
};

class FollowPath : public Function {
public:
	static shared_ptr<FollowPath> pathToTarget(GSpace* space, gobject_ref agent, gobject_ref target);

	FollowPath(GSpace* space, const ValueMap& args);
	inline FollowPath(Path path, bool loop) :
	path(path),
	loop(loop)
	{}

	virtual void update(StateMachine& fsm);
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(FollowPath)
protected:
	Path path;
	size_t currentTarget = 0;
	bool loop = false;
};

class Wander : public Function {
public:
    Wander(GSpace* space, const ValueMap& args);

    inline Wander() : minWait(1.0), maxWait(3.0), minDist(2.0), maxDist(4.0)
    {}

    virtual void update(StateMachine& sm);

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(Wander)
protected:
    SpaceFloat minWait, maxWait;
    SpaceFloat minDist, maxDist;
};

class Operation : public Function {
public:
    inline Operation(std::function<void(StateMachine&)> op) :
    op(op)
    {}

    inline virtual void update(StateMachine& sm){
        op(sm);
        sm.pop();
    }
    FuncGetName(Operation)
protected:
    std::function<void(StateMachine&)> op;
};

class Cast : public Function {
public:
    Cast(string _spell_name, const ValueMap& _spell_args);
    Cast(GSpace* space, const ValueMap& _spell_args);

    virtual void onEnter(StateMachine& sm);
    virtual void update(StateMachine& sm);
    virtual void onExit(StateMachine& sm);
    
    FuncGetName(Cast)

	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::spellcasting);
	}

protected:
    string spell_name;
    ValueMap spell_args;
};

class FireAtTarget : public Function {
public:
	FireAtTarget(shared_ptr<FirePattern> pattern, gobject_ref target);

	virtual void update(StateMachine& sm);

	FuncGetName(FireAtTarget)
	
	inline virtual bitset<lockCount> getLockMask() {
		return make_enum_bitfield(ResourceLock::fire);
	}

protected:
	shared_ptr<FirePattern> pattern;
	gobject_ref target;
};

class FacerMain : public Function {
public:
	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
    FuncGetName(FacerMain)
protected:
	gobject_ref target = nullptr;
};

class FollowerMain : public Function {
public:
	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
    FuncGetName(FollowerMain)
protected:
	gobject_ref target = nullptr;
};

class SakuyaMain : public Function {
public:
	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
    FuncGetName(SakuyaMain)
};

class IllusionDash : public Function {
public:
    IllusionDash(SpaceVect _target);
    IllusionDash(GSpace* space, const ValueMap& args);
    static const SpaceFloat scale;
    static const SpaceFloat opacity;
    static const SpaceFloat speed;

	virtual void onEnter(StateMachine& sm);
    virtual void update(StateMachine& sm);
    FuncGetName(IllusionDash)
protected:
    SpaceVect target;
};

} //end NS

#endif /* AI_hpp */
