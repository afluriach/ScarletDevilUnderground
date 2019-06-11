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

class Thread;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}
#define GetLockmask(l) inline virtual bitset<lockCount> getLockMask() { return make_enum_bitfield(ResourceLock::l); }
#define GetLockmask2(l, m) inline virtual bitset<lockCount> getLockMask() { return make_enum_bitfield(ResourceLock::l) | make_enum_bitfield(ResourceLock::m); }

class Function
{
public:
	friend class Thread;

	inline Function() {}
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
	static function<bool(pair<unsigned int, bullet_collide_function>)> isCallback(unsigned int id);

    GObject *const agent;

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
	unsigned int addBulletHitFunction(bullet_collide_function f);
	unsigned int addBulletBlockFunction(bullet_collide_function f);
	bool removeBulletFunction(unsigned int id);
	void setAlertFunction(alert_function f);

	//wrappers for the current thread
	void push(shared_ptr<Function> f);
	void pop();
    
	Agent* getAgent();
	RoomSensor* getRoomSensor();
	unsigned int getFrame();
	Thread* getCrntThread();
    string toString();
protected:
	void applyAddThreads();
	void applyRemoveThreads();
	void removeCompletedThreads();

	unordered_set<unsigned int> threadsToRemove;
	list<shared_ptr<Thread>> threadsToAdd;

	list<pair<unsigned int, bullet_collide_function>> bulletHitHandlers;
	list<pair<unsigned int, bullet_collide_function>> bulletBlockHandlers;
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

} //end NS

#endif /* AI_hpp */
