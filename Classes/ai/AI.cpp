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
#include "AIUtil.hpp"
#include "AreaSensor.hpp"
#include "FirePattern.hpp"
#include "LuaAPI.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"
#include "value_map.hpp"

namespace ai{

update_return::update_return() :
	update_return(0, nullptr)
{
}

update_return::update_return(int idx, local_shared_ptr<Function> f) :
	idx(idx),
	f(f)
{
}

bool update_return::isSteady()
{
	return idx == 0;
}

bool update_return::isPop()
{
	return idx < 0;
}

Function::Function(StateMachine* fsm) :
	fsm(fsm)
{}

Function::~Function()
{
}

void Function::pop() {
	if (thread)
		thread->pop();
	else
		log("Function::pop: %s is not stackful!", getName());
}

GSpace* Function::getSpace() const {
	return fsm->getSpace();
}

GObject* Function::getObject() const {
	return fsm->getObject();
}

Agent* Function::getAgent() const {
	return fsm->getAgent();
}

physics_context* Function::getPhys() const {
	return getSpace()->physicsContext.get();
}

bool Function::fire()
{
	Agent* agent = getAgent();
	FirePattern* fp = agent->getFirePattern();
	bool fired = false;

	if (!fp) {
		log("%s: Attempt to fire without FirePattern!", getObject()->toString());
		return false;
	}

	fired = fp->fireIfPossible();
	if (fired) {
		agent->playSoundSpatial("sfx/shot.wav");
	}

	return fired;
}

bool Function::aimAtTarget(gobject_ref target)
{
	GObject* agent = getObject();

	if (!target.isValid())
		return false;

	agent->setAngle(directionToTarget(agent, target.get()->getPos()).toAngle());
	return true;
}

bool Function::castSpell(local_shared_ptr<SpellDesc> desc)
{
	if (isSpellActive())
		stopSpell();

	spellID = getObject()->cast(desc);
	return spellID != 0;
}

unsigned int Function::castSpellManual(local_shared_ptr<SpellDesc> desc)
{
	return getObject()->cast(desc);
}

bool Function::isSpellActive()
{
	return spellID != 0 && getSpace()->spellSystem->isSpellActive(spellID);
}

void Function::stopSpell()
{
	getSpace()->spellSystem->stopSpell(spellID);
	spellID = 0;
}

Thread::Thread(local_shared_ptr<Function> threadMain, StateMachine* sm) :
	sm(sm)
{
	if (threadMain) {
		push(threadMain);
	}
}

Thread::~Thread()
{
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

	for (int i = 0; i < -result.idx && !call_stack.empty(); ++i) {
		pop();
	}

	if (result.f.get()) {
		push(result.f);
	}
}

void Thread::push(local_shared_ptr<Function> newState)
{
	newState->thread = this;
	call_stack.push_back(newState);
}

void Thread::pop()
{
	if (call_stack.empty())
		return;

	call_stack.back()->thread = nullptr;
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

local_shared_ptr<Function> Thread::getTop()
{
	if (call_stack.size() > 0)
		return call_stack.back();
	else
		return nullptr;
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

StateMachine::~StateMachine()
{
}

bool StateMachine::runScriptPackage(const string& name)
{
	sol::function p = GSpace::scriptVM->getFunction(name);

	if (p) {
		try {
			p(this);
		}
		catch (const sol::error& e) {
			log("script package %s error: %s", name, e.what());
			return false;
		}
		return true;
	}
	else {
		return false;
	}
}

void StateMachine::update()
{
	removeCompletedThreads();
    
    for(auto thread_it = current_threads.rbegin(); thread_it != current_threads.rend(); ++thread_it)
    {
		(*thread_it)->update();
    }
}

void StateMachine::addFunction(local_shared_ptr<Function> function)
{
	if (!function) return;

	event_type events = function->getEvents();
	if (events != event_type::none) {
		functions.push_back(make_pair(events, function));
	}
	else {
		log("StateMachine::addFunction: Attempt to add function %s that doesn't handle any events.", function->getName());
	}
}

void StateMachine::removeFunction(local_shared_ptr<Function> function)
{
	if (!function) return;

	event_type events = function->getEvents();
	functions.remove(make_pair(events, function));
}

local_shared_ptr<Thread> StateMachine::addThread(local_shared_ptr<Thread> thread)
{
	current_threads.push_back(thread);
	return thread;
}

local_shared_ptr<Thread> StateMachine::addThread(local_shared_ptr<Function> threadMain)
{
	auto t = make_local_shared<Thread>(threadMain,this);

   addThread(t);
   return t;
}

void StateMachine::removeThread(local_shared_ptr<Thread> t)
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
	callInterface<GObject*>(ai::event_type::detect, &ai::Function::detect, obj);
}

void StateMachine::onEndDetect(GObject* obj)
{
	callInterface<GObject*>(ai::event_type::detect, &ai::Function::endDetect, obj);
}

void StateMachine::onBulletHit(Bullet* b)
{
	callInterface<Bullet*>(ai::event_type::bulletHit, &ai::Function::bulletHit, b);
}

void StateMachine::onBulletBlock(Bullet* b)
{
	callInterface<Bullet*>(ai::event_type::bulletBlock, &ai::Function::bulletBlock, b);
}

void StateMachine::onAlert(Player* p)
{
	callInterface<Player*>(ai::event_type::roomAlert, &ai::Function::roomAlert, p);
}

void StateMachine::onZeroHP()
{
	callInterface(ai::event_type::zeroHP, &ai::Function::zeroHP);
}

void StateMachine::onZeroStamina()
{
	callInterface(ai::event_type::zeroStamina, &ai::Function::zeroStamina);
}

void StateMachine::addOnDetectHandler(GType type, AITargetFunctionGenerator gen)
{
	auto detect = make_local_shared<OnDetect>(this, type, gen);
	addFunction(detect);
}

void StateMachine::addWhileDetectHandler(GType type, AITargetFunctionGenerator gen)
{
	auto detect = make_local_shared<WhileDetect>(this, type, gen);
	addFunction(detect);
}

void StateMachine::addFleeBomb()
{
	addWhileDetectHandler(GType::bomb, makeTargetFunctionGenerator<Flee>(-1.0));
}

void StateMachine::addAlertHandler(AITargetFunctionGenerator gen)
{
	auto alert = make_local_shared<OnAlert>(this, gen);
	addFunction(alert);
}

void StateMachine::addAlertFunction(alert_function f)
{
	auto alert = make_local_shared<OnAlertFunction>(this, f);
	addFunction(alert);
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

}//end NS
