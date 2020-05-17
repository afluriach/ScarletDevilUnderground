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
class SpellDesc;
namespace Lua { class Inst; }

namespace ai{

class Function;
class OnDetectFunction;

enum class event_type
{
	begin = 0,
	bulletBlock = 0,
	bulletHit,

	detect,
	endDetect,

	roomAlert,

	zeroHP,
	zeroStamina,

	end
};

constexpr size_t eventCount = to_size_t(event_type::end);

typedef bitset<eventCount> event_bitset;
typedef pair<event_bitset, local_shared_ptr<Function>> function_entry;

//for functions that target an object
typedef function<local_shared_ptr<Function>(StateMachine*, GObject*)> AITargetFunctionGenerator;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}
#define GetLockmask(l) inline virtual lock_mask getLockMask() { return make_enum_bitfield(ResourceLock::l); }
#define GetLockmask2(l, m) inline virtual lock_mask getLockMask() { return make_enum_bitfield(ResourceLock::l) | make_enum_bitfield(ResourceLock::m); }

#define return_pop() return update_return(-1, nullptr)
#define return_push(x) return update_return(0, x)
#define _steady() update_return(0, nullptr)
#define return_steady() return _steady()

#define return_pop_if_false(x) return update_return( (x) ? 0 : -1, nullptr)
#define return_pop_if_true(x) return update_return( (x) ? -1 : 0, nullptr)

#define return_push_if_true(x, s) return update_return( 0, (x) ? (s) : nullptr )
#define return_push_if_valid(x) return update_return( 0, (x) ? (x) : nullptr )

struct update_return
{
	update_return();
	update_return(int idx, local_shared_ptr<Function> f);

	int idx;
	local_shared_ptr<Function> f;
};

class Event
{
public:
	Event(event_type eventType, any data);

	bool isBulletHit();
	bool isDetectPlayer();
	//returns none type if event is not a detection
	GType getDetectType();
	GType getEndDetectType();

	Player* getRoomAlert();

	inline event_type getEventType() const { return eventType; }

	event_type eventType;
	any data;
};

class Function
{
public:
	friend class Thread;

	template<class C>
	inline static void autoUpdateFunction(local_shared_ptr<C>& f) {
		if (f && !f->isCompleted())
			f->update();
		if (f && f->isCompleted())
			f.reset();
	}

	Function(StateMachine* fsm);
	virtual ~Function();

	inline StateMachine* getFSM() const { return fsm; }

	template<class FuncCls, typename... Params>
	void push(Params... params);
	void pop();

	GSpace* getSpace() const;
	GObject* getObject() const;
	Agent* getAgent() const;
	physics_context* getPhys() const;

	bool fire();
	bool aimAtTarget(gobject_ref target);

	bool castSpell(local_shared_ptr<SpellDesc> desc);
	bool isSpellActive();
	void stopSpell();

	inline virtual void onEnter() {}
    inline virtual void onReturn() {}
	inline virtual void reset() {}
	inline virtual update_return update() { return_pop(); }
	inline virtual bool isActive() { return false; }
	inline virtual bool isCompleted() { return false; }
	inline virtual void onExit() {}
    
	inline virtual bool onEvent(Event event) { return false; }
	inline virtual event_bitset getEvents() { return event_bitset(); }

    inline virtual string getName() const {return "Function";}
    
	StateMachine* const fsm;
protected:
	unsigned int spellID = 0;
	Thread* thread = nullptr;
	bool hasRunInit = false;
};

class Thread
{
public:    
	friend class StateMachine;
    
	Thread(local_shared_ptr<Function> threadMain, StateMachine* sm);
	~Thread();

	void update();

	void push(local_shared_ptr<Function> newState);
	void pop();
	void popToRoot();
    
	local_shared_ptr<Function> getTop();
    string getStack();
	string getMainFuncName();
protected:
	list<local_shared_ptr<Function>> call_stack;
	StateMachine* sm;
};

class StateMachine
{
public:
	friend class Thread;

	typedef function<void(StateMachine*, const ValueMap&) > PackageType;
	static const unordered_map<string, StateMachine::PackageType> packages;

    StateMachine(GObject *const agent);
	~StateMachine();

	bool runScriptPackage(const string& name);

	void update();

	void addFunction(local_shared_ptr<Function> function);
	void removeFunction(local_shared_ptr<Function> function);
	local_shared_ptr<Thread> addThread(local_shared_ptr<Thread> thread);
    local_shared_ptr<Thread> addThread(local_shared_ptr<Function> threadMain);
    void removeThread(local_shared_ptr<Thread> thread);
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
	void onZeroStamina();

	void addAlertHandler(AITargetFunctionGenerator gen);
	void addOnDetectHandler(GType type, AITargetFunctionGenerator gen);
	void addWhileDetectHandler(GType type, AITargetFunctionGenerator gen);
	void addFleeBomb();

	void addDetectFunction(GType t, detect_function begin, detect_function end);
	void addAlertFunction(alert_function f);

	template<class FuncCls, typename... Params>
	inline void addFunction(Params... params) {
		addFunction(make_local_shared<FuncCls>(this, params...));
	}

	template<class FuncCls, typename... Params>
	inline local_shared_ptr<Thread> addThread(Params... params) {
		return addThread(make_local_shared<FuncCls>(this, params...));
	}

	template<class FuncCls, typename... Params>
	inline local_shared_ptr<FuncCls> make(Params... params) {
		return make_local_shared<FuncCls>(this, params...);
	}
    
	GSpace* getSpace();
	inline GObject* getObject() { return agent; }
	Agent* getAgent();
	RoomSensor* getRoomSensor();
	unsigned int getFrame();
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
			if ((f.get()->*method)(params...))
				return true;
		}
		return false;
	}

	void removeCompletedThreads();
	void handleEvent(Event event);

	GObject *const agent;

	list<local_shared_ptr<Thread>> current_threads;
	list<function_entry> functions;
    unsigned int frame;
};

template<class FuncCls, typename... Params>
inline void Function::push(Params... params) {
	if (thread) {
		thread->push(make_local_shared<FuncCls>(fsm, params...));
	}
	else {
		log("Function::push: %s is not stackful!", getName());
	}
}

template<class FuncCls, typename... Params>
inline AITargetFunctionGenerator makeTargetFunctionGenerator(Params... params)
{
	return[params...](StateMachine* fsm, GObject* obj)->local_shared_ptr<Function> {
		return make_local_shared<FuncCls>(fsm, obj, params...);
	};
}

} //end NS

#endif /* AI_hpp */
