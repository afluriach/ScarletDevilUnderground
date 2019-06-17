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
#include "AreaSensor.hpp"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "value_map.hpp"

namespace ai{

shared_ptr<Function> Function::constructState(
	const string& type,
	StateMachine* fsm,
	const ValueMap& args
){
	Function::AdapterType adapter = getOrDefault(Function::adapters, type, Function::AdapterType());

	if (adapter)
		return adapter(fsm, args);
	else
		return nullptr;
}

Function::Function(StateMachine* fsm) :
	fsm(fsm),
	agent(fsm->getAgent())
{}

void Function::pop() {
	fsm->pop();
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

void Thread::update()
{
	if (call_stack.empty()) {
		completed = true;
		return;
	}

	Function* crnt = call_stack.back().get();

	if (!crnt->hasRunInit) {
		crnt->onEnter();
		crnt->hasRunInit = true;
	}

	update_return result = crnt->update();

	for (int i = 0; i < -result.first && !call_stack.empty(); ++i) {
		pop();
	}

	if (result.second.get()) {
		push(result.second);
	}
}

void Thread::onDelay()
{
    if(resetOnBlock)
    {
        while(call_stack.size() > 1){
            pop();
        }
    }
}

bool Thread::onBulletHit(Bullet* b)
{
	return callInterface(&Function::onBulletHit, b);
}

bool Thread::onBulletBlock(Bullet* b)
{
	return callInterface(&Function::onBulletBlock, b);
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

	crnt->onExit();
	call_stack.pop_back();

	if (!call_stack.empty())
		call_stack.back()->onReturn();
}

shared_ptr<Function> Thread::getTop()
{
	return call_stack.back();
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
                
                crntThread->update();
            }
            else
            {
                //Call the thread onDelay if there was a conflict with the thread
                //lock bits.
                if((locks & crntThread->lockMask).any()){
                    crntThread->onDelay();
                }
                if((locks & crntThread->call_stack.back()->getLockMask()).any()){
                    crntThread->call_stack.back()->onDelay();
                }
            }
            crntThread = nullptr;
        }
    }
}

void StateMachine::addThread(shared_ptr<Thread> thread)
{
	if (!thread->call_stack.empty()) {
		threadsToAdd.push_back(thread);
		log("%s: FSM creating Thread %s.", agent->getName().c_str(), thread->call_stack.back()->getName());
	}
	else {
		log("%s: FSM creating empty Thread.", agent->getName().c_str());
	}
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

		emplaceIfEmpty(threads_by_priority, thread->priority);
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

int StateMachine::getThreadCount()
{
	return current_threads.size();
}

void StateMachine::applyRemoveThreads()
{
	for (unsigned int uuid : threadsToRemove)
	{
		if (current_threads.find(uuid) != current_threads.end()) {
			auto t = current_threads[uuid];
			if (!t->call_stack.empty()) {
				log("%s: FSM removing Thread %s.", agent->getName().c_str(), t->call_stack.back()->getName());
			}
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
	callInterface(&Function::onBulletHit, b);
}

void StateMachine::onBulletBlock(Bullet* b)
{
	callInterface(&Function::onBulletBlock, b);
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

void StateMachine::setAlertFunction(alert_function f)
{
	alertHandler = f;
}

void StateMachine::push(shared_ptr<Function> f)
{
	crntThread->push(f);
}

void StateMachine::pop()
{
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

GSpace* StateMachine::getSpace() {
	return agent->space;
}

Agent* StateMachine::getAgent() {
	return dynamic_cast<Agent*>(agent);
}

RoomSensor* StateMachine::getRoomSensor() {
	return dynamic_cast<RoomSensor*>(agent);
}

unsigned int StateMachine::getFrame() {
	return frame;
}

Thread* StateMachine::getCrntThread() {
	return crntThread;
}

}//end NS
