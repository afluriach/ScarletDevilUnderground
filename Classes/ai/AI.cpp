//
//  AI.cpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AI.hpp"
#include "AIMixins.hpp"
#include "App.h"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "macros.h"
#include "util.h"
#include "value_map.hpp"

namespace ai{

shared_ptr<Function> Function::constructState(const string& type, GSpace* space, const ValueMap& args)
{
    auto it = Function::adapters.find(type);

    if (it != Function::adapters.end()) {
        Function::AdapterType adapter = it->second;
        return adapter(space,args);
    }
    else return nullptr;
}

unsigned int Thread::nextUUID = 1;

Thread::Thread(shared_ptr<Function> threadMain, StateMachine* sm) :
Thread(threadMain, sm, 0, bitset<lockCount>())
{
}

Thread::Thread(
    shared_ptr<Function> threadMain,
    StateMachine* sm,
    priority_type priority,
    bitset<lockCount> lockMask
) :
uuid(nextUUID++),
priority(priority),
lockMask(lockMask),
sm(sm)
{
	if (!threadMain) {
		log("thread created with null main!");
		completed = true;
	}
	else {
		push(threadMain);
	}
}

void Thread::update(StateMachine& sm)
{
	if (call_stack.empty()) {
		completed = true;
		return;
	}

	Function* crnt = call_stack.back().get();

	if (!crnt->hasRunInit) {
		crnt->onEnter(sm);
		crnt->hasRunInit = true;
	}

	crnt->update(sm);
}

void Thread::onDelay(StateMachine& sm)
{
    if(resetOnBlock)
    {
        while(call_stack.size() > 1){
            pop();
        }
    }
}

void Thread::push(shared_ptr<Function> newState)
{
	call_stack.push_back(newState);
}

void Thread::pop()
{
	if (call_stack.empty())
		return;
	Function* crnt = call_stack.back().get();

	crnt->onExit(*sm);
	call_stack.pop_back();

	if (!call_stack.empty())
		call_stack.back()->onReturn(*sm);
}

string Thread::getStack()
{
    stringstream ss;
    
    if(call_stack.size() == 0)
        return "<empty>";
    
    auto it = call_stack.begin();
    size_t i = 0;
    for(; i < call_stack.size()-1; ++i, ++it){
        ss << (*it)->getName() << " -> ";
    }
    ss << call_stack.back()->getName();
    
    return ss.str();
}

string Thread::getMainFuncName() {
	return call_stack.front()->getName();
}

void Thread::setResetOnBlock(bool reset) {
	resetOnBlock = reset;
}

function<bool(pair<unsigned int, bullet_collide_function>)> StateMachine::isCallback(unsigned int id)
{
	return [id](pair<unsigned int, bullet_collide_function> entry) -> bool {
		return entry.first == id;
	};
}

StateMachine::StateMachine(GObject *const agent) :
agent(agent)
{
    frame = agent->space->getFrame();
}

void StateMachine::update()
{
    bitset<lockCount> locks;
    
	removeCompletedThreads();
	applyRemoveThreads();
	applyAddThreads();
    
    for(auto priority_it = threads_by_priority.rbegin(); priority_it != threads_by_priority.rend(); ++priority_it)
    {
        for(unsigned int uuid: priority_it->second)
        {
            crntThread = current_threads[uuid].get();
            
            //Take union of thread locks & current function locks
            bitset<lockCount> lockMask = crntThread->lockMask | crntThread->call_stack.back()->getLockMask();
            
            if(!(locks & lockMask).any() )
            {
                //The current function in this thread does not require a lock that has
                //already been acquired this frame.
                
                locks |= lockMask;
                
                crntThread->update(*this);
            }
            else
            {
                //Call the thread onDelay if there was a conflict with the thread
                //lock bits.
                if((locks & crntThread->lockMask).any()){
                    crntThread->onDelay(*this);
                }
                if((locks & crntThread->call_stack.back()->getLockMask()).any()){
                    crntThread->call_stack.back()->onDelay(*this);
                }
            }
            crntThread = nullptr;
        }
    }
}

void StateMachine::addThread(shared_ptr<Thread> thread)
{
	threadsToAdd.push_back(thread);
}

unsigned int StateMachine::addThread(shared_ptr<Function> threadMain, Thread::priority_type priority)
{
	auto t = make_shared<Thread>(
		threadMain,
		this,
		priority,
		bitset<lockCount>()
	);

   addThread(t);
   return t->uuid;
}

void StateMachine::applyAddThreads()
{
	for (shared_ptr<Thread> thread : threadsToAdd)
	{
		current_threads[thread->uuid] = thread;

		if (threads_by_priority.find(thread->priority) == threads_by_priority.end()) {
			threads_by_priority[thread->priority] = list<unsigned int>();
		}
		threads_by_priority[thread->priority].push_back(thread->uuid);
	}
	threadsToAdd.clear();
}

void StateMachine::removeThread(unsigned int uuid)
{
	threadsToRemove.insert(uuid);
}

void StateMachine::removeThread(const string& mainName)
{
    for(auto it = current_threads.begin(); it != current_threads.end(); ++it)
    {
        if(it->second->getMainFuncName() == mainName){
            threadsToRemove.insert(it->second->uuid);
        }
    }
}

void StateMachine::removeCompletedThreads()
{
    vector<unsigned int> toRemove;
    
    for(auto it = current_threads.begin(); it != current_threads.end(); ++it){
        if(it->second->completed || it->second->call_stack.empty()){
            toRemove.push_back(it->first);
        }
    }
    
    for(unsigned int uuid: toRemove){
        removeThread(uuid);
    }
}

bool StateMachine::isThreadRunning(const string& mainName)
{
	for (auto it = current_threads.begin(); it != current_threads.end(); ++it)
	{
		if (it->second->getMainFuncName() == mainName) return true;
	}
	return false;
}

void StateMachine::applyRemoveThreads()
{
	for (unsigned int uuid : threadsToRemove)
	{
		if (current_threads.find(uuid) != current_threads.end()) {
			auto t = current_threads[uuid];
			threads_by_priority[t->priority].remove(uuid);
			current_threads.erase(uuid);
		}
	}
	threadsToRemove.clear();
}

void StateMachine::onDetect(GObject* obj)
{
	auto it = detectHandlers.find(obj->getType());
	if (it != detectHandlers.end()) {
		it->second(*this, obj);
	}
}
void StateMachine::onEndDetect(GObject* obj)
{
	auto it = endDetectHandlers.find(obj->getType());
	if (it != endDetectHandlers.end()) {
		it->second(*this, obj);
	}
}

void StateMachine::onBulletHit(Bullet* b)
{
	for (auto entry : bulletHitHandlers) {
		entry.second(*this, b);
	}
}

void StateMachine::onBulletBlock(Bullet* b)
{
	for (auto entry : bulletBlockHandlers) {
		entry.second(*this, b);
	}
}

void StateMachine::onAlert(Player* p)
{
	if (!alerted && alertHandler) {
		alertHandler(*this, p);
	}
	alerted = true;
}


void StateMachine::addDetectFunction(GType t, detect_function f)
{
	detectHandlers.insert_or_assign(t, f);
}

void StateMachine::addEndDetectFunction(GType t, detect_function f)
{
	endDetectHandlers.insert_or_assign(t, f);
}

void StateMachine::removeDetectFunction(GType t)
{
	detectHandlers.erase(t);
}

void StateMachine::removeEndDetectFunction(GType t)
{
	endDetectHandlers.erase(t);
}

unsigned int StateMachine::addBulletHitFunction(bullet_collide_function f)
{
	auto entry = make_pair(nextCallbackID++, f);
	bulletHitHandlers.push_back(entry);
	return nextCallbackID - 1;
}
unsigned int StateMachine::addBulletBlockFunction(bullet_collide_function f)
{
	auto entry = make_pair(nextCallbackID++, f);
	bulletBlockHandlers.push_back(entry);
	return nextCallbackID - 1;
}

bool StateMachine::removeBulletFunction(unsigned int id)
{
	auto f = isCallback(id);
	size_t _prev = bulletHitHandlers.size() + bulletBlockHandlers.size();

	bulletHitHandlers.remove_if(f);
	bulletBlockHandlers.remove_if(f);

	return bulletHitHandlers.size() + bulletBlockHandlers.size() != _prev;
}

void StateMachine::setAlertFunction(alert_function f)
{
	alertHandler = f;
}

void StateMachine::push(shared_ptr<Function> f)
{
	log("%s FSM pushing %s.", agent->getName().c_str(), f->getName().c_str());
	crntThread->push(f);
}

void StateMachine::pop()
{
	log("%s FSM popping %s.", agent->getName().c_str(), crntThread->call_stack.back()->getName().c_str());
	crntThread->pop();
}

string StateMachine::toString()
{
    stringstream ss;
    
    ss << "StateMachine for " << agent->getName() << ":\n";
    
    for(auto it = current_threads.begin(); it != current_threads.end(); ++it)
    {
        Thread* t = it->second.get();
        ss << "thread " << t->uuid << ", pri " << t->priority << ", stack:  " << t->getStack() << "\n";
    }
    return ss.str();
}

Agent* StateMachine::getAgent() {
	return dynamic_cast<Agent*>(agent);
}

unsigned int StateMachine::getFrame() {
	return frame;
}

Thread* StateMachine::getCrntThread() {
	return crntThread;
}

}//end NS
