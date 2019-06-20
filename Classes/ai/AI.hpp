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

class Agent;
class GObject;
class GSpace;
class RoomSensor;

namespace ai{

enum class ResourceLock
{
    begin = 0,
    movement = 0,
	look,
	spellcasting,
	fire,
	shield,
	bomb,
    
    end,
};

constexpr size_t lockCount = to_size_t(ResourceLock::end);

class Function;
class StateMachine;
class Thread;

typedef pair<int, shared_ptr<Function>> update_return;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}
#define GetLockmask(l) inline virtual bitset<lockCount> getLockMask() { return make_enum_bitfield(ResourceLock::l); }
#define GetLockmask2(l, m) inline virtual bitset<lockCount> getLockMask() { return make_enum_bitfield(ResourceLock::l) | make_enum_bitfield(ResourceLock::m); }

#define return_pop() return make_pair(-1, nullptr)
#define return_push(x) return make_pair(0, x)
#define _steady() make_pair(0, nullptr)
#define return_steady() return _steady()

#define return_pop_if_false(x) return make_pair( (x) ? 0 : -1, nullptr)
#define return_pop_if_true(x) return make_pair( (x) ? -1 : 0, nullptr)

#define return_push_if_true(x, s) return make_pair( 0, (x) ? (s) : nullptr )
#define return_push_if_valid(x) return make_pair( 0, (x) ? (x) : nullptr )

class Function
{
public:
	friend class Thread;

	Function(StateMachine* fsm);
	inline virtual ~Function() {}

    typedef function<shared_ptr<Function>(StateMachine* fsm, const ValueMap&) > AdapterType;
    static const unordered_map<string, Function::AdapterType> adapters;
    
    static shared_ptr<Function> constructState(
		const string& type,
		StateMachine* fsm,
		const ValueMap& args
	);

	template<class FuncCls, typename... Params>
	void push(Params... params);
	void pop();

	inline virtual void onEnter() {}
    inline virtual void onReturn() {}
	//Returns this to continue running, nullptr to pop, and other to push
	inline virtual update_return update() { return_pop(); }
	inline virtual void onExit() {}
    
    inline virtual void onDelay() {}
    
	inline virtual bool onBulletHit(Bullet* b) { return false; }
	inline virtual bool onBulletBlock(Bullet* b) { return false; }

    inline virtual string getName() const {return "Function";}
    
    inline virtual bitset<lockCount> getLockMask() { return bitset<lockCount>();}
protected:
	StateMachine *const fsm;
	Agent *const agent;
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

	void update();
    void onDelay();

	bool onBulletHit(Bullet* b);
	bool onBulletBlock(Bullet* b);

	void push(shared_ptr<Function> newState);
	void pop();
    
	shared_ptr<Function> getTop();
    string getStack();
	string getMainFuncName();
	void setResetOnBlock(bool reset);

protected:
	//Calls a particular Function interface method, starting with the top of the stack,
	//and continuing until a handler returns to indicate it handled the event.
	//This function in turns returns whether any function in the stack handled the event.
	template<typename... Params>
	bool callInterface(bool (Function::*method)(Params...), Params... params)
	{
		for (auto it = call_stack.rbegin(); it != call_stack.rend(); ++it) {
			Function* f = it->get();
			if ( (f->*method)(params...))
				return true;
		}
		return false;
	}

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
    StateMachine(GObject *const agent);

	void update();

	void addThread(shared_ptr<Thread> thread);
    unsigned int addThread(shared_ptr<Function> threadMain, Thread::priority_type priority = 0);
    void removeThread(unsigned int uuid);
    //Remove thread(s) that have the given main function.
    void removeThread(const string& mainName);
	bool isThreadRunning(const string& mainName);
	int getThreadCount();

    void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	void onBulletHit(Bullet* b);
	void onBulletBlock(Bullet* b);
	void onAlert(Player* p);

	void addDetectFunction(GType t, detect_function f);
	void addEndDetectFunction(GType t, detect_function f);
	void removeDetectFunction(GType t);
	void removeEndDetectFunction(GType t);
	void setAlertFunction(alert_function f);

	template<class FuncCls, typename... Params>
	inline shared_ptr<FuncCls> make(Params... params) {
		return make_shared<FuncCls>(this, params...);
	}

	//wrappers for the current thread
	template<class FuncCls, typename... Params>
	inline void push(Params... params) {
		push(make<FuncCls>(params...));
	}
	void push(shared_ptr<Function> f);
	void pop();
    
	GSpace* getSpace();
	inline GObject* getObject() { return agent; }
	Agent* getAgent();
	RoomSensor* getRoomSensor();
	unsigned int getFrame();
	Thread* getCrntThread();
    string toString();
protected:
	//Calls a particular Function interface method, using Thread::callInterface,
	//iterating through Threads in order of descending priority,
	//until one of them handles the event.
	template<typename... Params>
	bool callInterface(bool (Function::*method)(Params...), Params... params)
	{
		for (auto priority_it = threads_by_priority.rbegin(); priority_it != threads_by_priority.rend(); ++priority_it)
		{
			for (unsigned int uuid : priority_it->second)
			{
				Thread* crnt = current_threads[uuid].get();

				if (crnt->callInterface(method, params...))
					return true;
			}
		}
		return false;
	}

	void applyAddThreads();
	void applyRemoveThreads();
	void removeCompletedThreads();

	GObject *const agent;

	unordered_set<unsigned int> threadsToRemove;
	list<shared_ptr<Thread>> threadsToAdd;

	alert_function alertHandler;
	unordered_map<GType, detect_function> detectHandlers;
	unordered_map<GType, detect_function> endDetectHandlers;

	map<unsigned int,shared_ptr<Thread>> current_threads;
    map<int, list<unsigned int>> threads_by_priority;
    unsigned int frame;
	unsigned int nextCallbackID = 1;
	Thread* crntThread = nullptr;
	bool alerted = false;
};

template<class FuncCls, typename... Params>
inline void Function::push(Params... params) {
	fsm->push<FuncCls>(params...);
}

} //end NS

#endif /* AI_hpp */
