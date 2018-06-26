//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

#include "object_ref.hpp"
#include "types.h"

class Agent;
class GObject;

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject& obj, const SpaceVect& desired, float maxForce);

void seek(GObject& agent, const GObject& target, float maxSpeed, float acceleration);
void seek(GObject& agent, const SpaceVect& target, float maxSpeed, float acceleration);

void flee(GObject& agent, const GObject& target, float maxSpeed, float acceleration);
void flee(GObject& agent, const SpaceVect& target, float maxSpeed, float acceleration);

bool isFacingTarget(const GObject& agent, const GObject& target);
bool isFacingTargetsBack(const GObject& agent, const GObject& target);
bool isLineOfSight(const GObject& agent, const GObject& target);

SpaceVect directionToTarget(const GObject& agent, const GObject& target);
SpaceVect directionToTarget(const GObject& agent, const SpaceVect& target);

SpaceVect displacementToTarget(const GObject& agent, const SpaceVect& target);
SpaceVect displacementToTarget(const GObject& agent, const GObject& target);

float distanceToTarget(const GObject& agent, const GObject& target);

float viewAngleToTarget(const GObject& agent, const GObject& target);

class StateMachine;

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

	inline virtual void onDetect(StateMachine& sm, GObject* obj) {}
	inline virtual void onEndDetect(StateMachine& sm, GObject* obj) {}
};

class Thread
{
public:
	friend class StateMachine;

	Thread(shared_ptr<Function> threadMain);

	void update(StateMachine& sm);

	void push(shared_ptr<Function> newState);
	void pop();

	void onDetect(StateMachine& sm, GObject* obj);
	void onEndDetect(StateMachine& sm, GObject* obj);

	inline bool isCompleted() {
		return completed;
	}

protected:
	list<shared_ptr<Function>> call_stack;
	StateMachine* sm;
	bool completed = false;
};

class StateMachine
{
public:
    GObject *const agent;

    StateMachine(GObject *const agent);

	void update();

	void addThread(shared_ptr<Function> threadMain);
	void removeCompletedThreads();

    void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	//wrappers for the current thread
	void push(shared_ptr<Function> f);
	void pop();
    
    inline unsigned int getFrame(){
        return frame;
    }

	inline Thread* getCrntThread() {
		return crntThread;
	}
    
protected:
	list<shared_ptr<Thread>> current_threads;
    unsigned int frame;
	Thread* crntThread = nullptr;
};

class Detect : public Function{
public:
    typedef function<shared_ptr<Function>(GObject* detected)> Generator;

    inline Detect(const string& target_name, Generator nextState) :
    target_name(target_name),
    nextState(nextState)
    {}

    virtual void onDetect(StateMachine& sm, GObject* obj);
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
protected:
	gobject_ref target;
};

class MaintainDistance : public Function {
public:
    inline MaintainDistance(gobject_ref target, float distance, float margin) :
    target(target),
    distance(distance),
    margin(margin)
    {}
    
	virtual void update(StateMachine& sm);
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
protected:
	gobject_ref target;
    float distance;
};

class DetectAndSeekPlayer : public Detect{
public:
    inline DetectAndSeekPlayer() :
    Detect(
        "player",
        [](GObject* target) -> shared_ptr<Function>{
            return make_shared<Seek>(target);
        }
    )
    {}
    
};

class IdleWait : public Function{
    public:
        IdleWait(const ValueMap& args);

        inline IdleWait(unsigned int frames) :
        remaining(frames)
        {}
    
		virtual void update(StateMachine& fsm);
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
    
protected:
    std::function<void(StateMachine&)> op;
};

class Cast : public Function {
public:
    Cast(string _spell_name, const ValueMap& _spell_args);

    virtual void onEnter(StateMachine& sm);
    virtual void update(StateMachine& sm);
    virtual void onExit(StateMachine& sm);
protected:
    string spell_name;
    ValueMap spell_args;
};

class FacerMain : public Function {
public:
	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
protected:
	gobject_ref target = nullptr;
};

class FollowerMain : public Function {
public:
	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
protected:
	gobject_ref target = nullptr;
};

class SakuyaMain : public Function {
public:
	virtual void onEnter(StateMachine& sm);
	virtual void update(StateMachine& sm);
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
protected:
    SpaceVect target;
};

} //end NS

#endif /* AI_hpp */
