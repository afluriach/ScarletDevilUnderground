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

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject& obj, SpaceVect desired, float maxForce);
void seek(GObject& agent, SpaceVect target, float maxSpeed, float acceleration);
SpaceVect fleeDirection(GObject& agent, SpaceVect target);
void flee(GObject& agent, SpaceVect target, float maxSpeed, float acceleration);

bool isFacingTarget(const GObject& agent, const GObject& target);
bool isFacingTargetsBack(const GObject& agent, const GObject& target);
bool isLineOfSight(const GObject& agent, const GObject& target);

array<float, 4> obstacleFeelerQuad(GObject& agent, float distance);
array<float, 8> obstacleFeeler8(GObject& agent, float distance);
int chooseBestDirection(const array<float, 8>& feelers, float desired_angle, float min_distance);

SpaceVect directionToTarget(const GObject& agent, SpaceVect target);
SpaceVect displacementToTarget(const GObject& agent, SpaceVect target);
float distanceToTarget(const GObject& agent, const GObject& target);
float viewAngleToTarget(const GObject& agent, const GObject& target);
SpaceVect projectileEvasion(const GObject& bullet, const GObject& agent);

float getStoppingTime(float speed, float acceleration);
float getStoppingDistance(float speed, float accceleration);
float getTurningRadius(float speed, float acceleration);

enum class ResourceLock
{
    begin = 0,
    movement = 0,
    
    end,
};

constexpr size_t lockCount = to_size_t(ResourceLock::end);

class StateMachine;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}

class Function
{
public:

	inline virtual ~Function() {}

    typedef function<shared_ptr<Function>(const ValueMap&) > AdapterType;
    static const unordered_map<string, Function::AdapterType> adapters;
    
    static shared_ptr<Function> constructState(const string& type, const ValueMap& args);

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
	map<unsigned int,shared_ptr<Thread>> current_threads;
    map<int, list<unsigned int>> threads_by_priority;
    unsigned int frame;
	Thread* crntThread = nullptr;
};

class Detect : public Function{
public:
    typedef function<shared_ptr<Function>(GObject* detected)> Generator;

    Detect(const string& target_name, Generator nextState);
    Detect(const ValueMap& args);

    virtual void onDetect(StateMachine& sm, GObject* obj);
    
    FuncGetName(Detect)
protected:
    string target_name;
    Generator nextState;
};

class Seek : public Function {
public:
    inline Seek(GObject* target) : target(target)
    {}
    Seek(const ValueMap& args);
    
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
    MaintainDistance(gobject_ref target, float distance, float margin);
    MaintainDistance(const ValueMap& args);
    
	virtual void update(StateMachine& sm);
    
    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    
    FuncGetName(MaintainDistance)
protected:
	gobject_ref target;
    float distance, margin;
};

class Flee : public Function {
public:
    inline Flee(GObject* target, float distance) :
    target(target),
    distance(distance)
    {}
    Flee(const ValueMap& args);
    
	virtual void update(StateMachine& sm);
	virtual void onEndDetect(StateMachine& sm, GObject* target);

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    
    FuncGetName(Flee)
protected:
	gobject_ref target;
    float distance;
};

class EvadePlayerProjectiles : public Function {
public:
	EvadePlayerProjectiles();
	EvadePlayerProjectiles(const ValueMap& args);

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
    DetectAndSeekPlayer(const ValueMap& args);
    
    FuncGetName(DetectAndSeekPlayer)
};

class IdleWait : public Function{
    public:
        IdleWait(const ValueMap& args);

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

class MoveToPoint : public Function{
public:
    MoveToPoint(const ValueMap& args);

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
	FollowPath(const ValueMap& args);
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
    Wander(const ValueMap& args);

    inline Wander() : minWait(1.0), maxWait(3.0), minDist(2.0), maxDist(4.0)
    {}

    virtual void update(StateMachine& sm);

    inline virtual bitset<lockCount> getLockMask() {
        return make_enum_bitfield(ResourceLock::movement);
    }
    FuncGetName(Wander)
protected:
    float minWait, maxWait;
    float minDist, maxDist;
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
    Cast(const ValueMap& _spell_args);

    virtual void onEnter(StateMachine& sm);
    virtual void update(StateMachine& sm);
    virtual void onExit(StateMachine& sm);
    
    FuncGetName(Cast)
protected:
    string spell_name;
    ValueMap spell_args;
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
    IllusionDash(const ValueMap& args);
    static const float scale;
    static const float opacity;
    static const float speed;

	virtual void onEnter(StateMachine& sm);
    virtual void update(StateMachine& sm);
    FuncGetName(IllusionDash)
protected:
    SpaceVect target;
};

} //end NS

#endif /* AI_hpp */
