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
#include "AIFunctions.hpp"
#include "AIMixins.hpp"
#include "AreaSensor.hpp"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "value_map.hpp"

namespace ai{

Event::Event(event_type eventType, any data) :
	eventType(eventType),
	data(data)
{
}

bool Event::isDetectPlayer()
{
	return eventType == event_type::detect && any_cast<GObject*>(data)->getType() == GType::player;
}

shared_ptr<Function> Function::constructState(
	const string& type,
	StateMachine* fsm,
	const ValueMap& args
){
	Function::AdapterType adapter = getOrDefault(Function::adapters, type);

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

GSpace* Function::getSpace() const {
	return fsm->getSpace();
}

GObject* Function::getAgentObject() const {
	return fsm->getObject();
}

Agent* Function::getAgent() const {
	return fsm->getAgent();
}

physics_context* Function::getPhys() const {
	return getSpace()->physicsContext.get();
}

Thread::Thread(shared_ptr<Function> threadMain, StateMachine* sm) :
	sm(sm)
{
	if (threadMain) {
		push(threadMain);
	}
}

void Thread::update()
{
	Function* crnt = !call_stack.empty() ? call_stack.back().get() : nullptr;
	if (!crnt) return;

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

bool Thread::onEvent(Event event)
{
	return callInterface(&Function::onEvent, event);
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

void Thread::popToRoot()
{
	while (call_stack.size() > 1) {
		pop();
	}
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
	return !call_stack.empty() ? call_stack.front()->getName() : "";
}

StateMachine::StateMachine(GObject *const agent) :
agent(agent)
{
    frame = getSpace()->getFrame();
}

void StateMachine::update()
{
    bitset<lockCount> locks;
    
	removeCompletedThreads();
    
    for(auto thread_it = current_threads.rbegin(); thread_it != current_threads.rend(); ++thread_it)
    {
        crntThread = thread_it->get();
            
        bitset<lockCount> lockMask = crntThread->call_stack.back()->getLockMask();
            
        if(!(locks & lockMask).any() )
        {
            //The current function in this thread does not require a lock that has
            //already been acquired this frame.
                
            locks |= lockMask;
                
            crntThread->update();
        }
        crntThread = nullptr;
    }
}

void StateMachine::addFunction(shared_ptr<Function> function)
{
	functions.push_back(function);
}

void StateMachine::addThread(shared_ptr<Thread> thread)
{
	current_threads.push_back(thread);
}

shared_ptr<Thread> StateMachine::addThread(shared_ptr<Function> threadMain)
{
	auto t = make_shared<Thread>(threadMain,this);

   addThread(t);
   return t;
}

void StateMachine::removeThread(shared_ptr<Thread> t)
{
	current_threads.remove(t);
}

void StateMachine::removeThread(const string& mainName)
{
	auto it = current_threads.begin();
	while (it != current_threads.end()) {
		if ((*it)->getMainFuncName() == mainName) {
			it = current_threads.erase(it);
		}
		else {
			++it;
		}
	}
}

void StateMachine::removeCompletedThreads()
{
	auto it = current_threads.begin();
	while (it != current_threads.end()) {
		if ((*it)->call_stack.empty()) {
			it = current_threads.erase(it);
		}
		else {
			++it;
		}
	}
}

bool StateMachine::isThreadRunning(const string& mainName)
{
	for (auto it = current_threads.begin(); it != current_threads.end(); ++it)
	{
		if ( (*it)->getMainFuncName() == mainName) return true;
	}
	return false;
}

int StateMachine::getThreadCount()
{
	return current_threads.size();
}

void StateMachine::onDetect(GObject* obj)
{
	Event event(event_type::detect, make_any<GObject*>(obj));
	callInterface(&Function::onEvent, event);

	auto it = detectHandlers.find(obj->getType());
	if (it != detectHandlers.end()) {
		it->second(*this, obj);
	}
}
void StateMachine::onEndDetect(GObject* obj)
{
	Event event(event_type::endDetect, make_any<GObject*>(obj));
	callInterface(&Function::onEvent, event);

	auto it = endDetectHandlers.find(obj->getType());
	if (it != endDetectHandlers.end()) {
		it->second(*this, obj);
	}
}

void StateMachine::onBulletHit(Bullet* b)
{
	Event event(event_type::bulletHit, make_any<Bullet*>(b));

	callInterface(&Function::onEvent, event);
}

void StateMachine::onBulletBlock(Bullet* b)
{
	Event event(event_type::bulletBlock, make_any<Bullet*>(b));

	callInterface(&Function::onEvent, event);
}

void StateMachine::onAlert(Player* p)
{
	if (!alerted && alertHandler) {
		alertHandler(*this, p);
	}
	alerted = true;
}

void StateMachine::onZeroHP()
{
	Event event(event_type::zeroHP, any());

	callInterface(&Function::onEvent, event);
}

void StateMachine::addWhileDetectHandler(GType type, AITargetFunctionGenerator gen)
{
	auto detect = make_shared<WhileDetect>(this, type, gen);
	addThread(detect);
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
        Thread* t = it->get();
        ss << "thread " << t << ", stack:  " << t->getStack() << "\n";
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
