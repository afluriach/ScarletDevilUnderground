//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

class Agent;
class GObject;
class GSpace;
class physics_context;
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

typedef bitset<lockCount> lock_mask;
typedef pair<int, shared_ptr<Function>> update_return;

//for functions that target an object
typedef function<shared_ptr<Function>(StateMachine*, GObject*)> AITargetFunctionGenerator;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}
#define GetLockmask(l) inline virtual lock_mask getLockMask() { return make_enum_bitfield(ResourceLock::l); }
#define GetLockmask2(l, m) inline virtual lock_mask getLockMask() { return make_enum_bitfield(ResourceLock::l) | make_enum_bitfield(ResourceLock::m); }

#define return_pop() return make_pair(-1, nullptr)
#define return_push(x) return make_pair(0, x)
#define _steady() make_pair(0, nullptr)
#define return_steady() return _steady()

#define return_pop_if_false(x) return make_pair( (x) ? 0 : -1, nullptr)
#define return_pop_if_true(x) return make_pair( (x) ? -1 : 0, nullptr)

#define return_push_if_true(x, s) return make_pair( 0, (x) ? (s) : nullptr )
#define return_push_if_valid(x) return make_pair( 0, (x) ? (x) : nullptr )

enum class event_type
{
	bulletHit,
	bulletBlock,

	detect,
	endDetect,

	zeroHP,

	end
};

class Event
{
public:
	Event(event_type eventType, any data);

	bool isDetectPlayer();
	//returns none type if event is not a detection
	GType getDetectType();
	GType getEndDetectType();

	event_type eventType;
	any data;
};

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

	GSpace* getSpace() const;
	GObject* getAgentObject() const;
	Agent* getAgent() const;
	physics_context* getPhys() const;

	inline virtual void onEnter() {}
    inline virtual void onReturn() {}
	inline virtual update_return update() { return_pop(); }
	inline virtual void onExit() {}
    
	inline virtual bool onEvent(Event event) { return false; }

    inline virtual string getName() const {return "Function";}
    
    inline virtual lock_mask getLockMask() { return lock_mask();}
protected:
	StateMachine *const fsm;
	Agent *const agent;
	bool hasRunInit = false;
};

class Thread
{
public:    
	friend class StateMachine;
    
	Thread(shared_ptr<Function> threadMain, StateMachine* sm);

	void update();

	bool onEvent(Event event);

	void push(shared_ptr<Function> newState);
	void pop();
	void popToRoot();
    
	shared_ptr<Function> getTop();
    string getStack();
	string getMainFuncName();
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
};

class StateMachine
{
public:
	typedef function<void(StateMachine*, const ValueMap&) > PackageType;
	static const unordered_map<string, StateMachine::PackageType> packages;

    StateMachine(GObject *const agent);

	void update();

	void addFunction(shared_ptr<Function> function);
	void addThread(shared_ptr<Thread> thread);
    shared_ptr<Thread> addThread(shared_ptr<Function> threadMain);
    void removeThread(shared_ptr<Thread> thread);
    //Remove thread(s) that have the given main function.
    void removeThread(const string& mainName);
	bool isThreadRunning(const string& mainName);
	int getThreadCount();

    void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	void onBulletHit(Bullet* b);
	void onBulletBlock(Bullet* b);
	void onAlert(Player* p);
	void onZeroHP();

	void addWhileDetectHandler(GType type, AITargetFunctionGenerator gen);
	void addFleeBomb();

	void addDetectFunction(GType t, detect_function f);
	void addEndDetectFunction(GType t, detect_function f);
	void removeDetectFunction(GType t);
	void removeEndDetectFunction(GType t);
	void setAlertFunction(alert_function f);

	template<class FuncCls, typename... Params>
	inline void addFunction(Params... params) {
		addFunction(make_shared<FuncCls>(this, params...));
	}

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
		for (auto f : functions)
		{
			(f.get()->*method)(params...);
		}

		for (auto thread_it = current_threads.rbegin(); thread_it != current_threads.rend(); ++thread_it)
		{
			Thread* crnt = thread_it->get();

			if (crnt->callInterface(method, params...))
				return true;
		}
		return false;
	}

	void removeCompletedThreads();

	GObject *const agent;

	alert_function alertHandler;
	unordered_map<GType, detect_function> detectHandlers;
	unordered_map<GType, detect_function> endDetectHandlers;

	list<shared_ptr<Thread>> current_threads;
	list<shared_ptr<Function>> functions;
    unsigned int frame;
	Thread* crntThread = nullptr;
	bool alerted = false;
};

template<class FuncCls, typename... Params>
inline void Function::push(Params... params) {
	fsm->push<FuncCls>(params...);
}

template<class FuncCls, typename... Params>
inline AITargetFunctionGenerator makeTargetFunctionGenerator(Params... params)
{
	return[params...](StateMachine* fsm, GObject* obj)->shared_ptr<Function> {
		return make_shared<FuncCls>(fsm, obj, params...);
	};
}

} //end NS

#endif /* AI_hpp */
