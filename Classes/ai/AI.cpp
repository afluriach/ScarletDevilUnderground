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

Event::Event(event_type eventType, any data) :
	eventType(eventType),
	data(data)
{
}

bool Event::isBulletHit()
{
	return eventType == event_type::bulletHit;
}

bool Event::isDetectPlayer()
{
	return getDetectType() == GType::player;
}

GType Event::getDetectType()
{
	return eventType == event_type::detect ? any_cast<GObject*>(data)->getType() : GType::none;
}

GType Event::getEndDetectType()
{
	return eventType == event_type::endDetect ? any_cast<GObject*>(data)->getType() : GType::none;
}

Player* Event::getRoomAlert()
{
	return eventType == event_type::roomAlert ? any_cast<Player*>(data) : nullptr;
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
	if (spellID != 0)
		stopSpell();

	spellID = getObject()->cast(desc);
	return spellID != 0;
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
	sm->addFunction(newState);
	call_stack.push_back(newState);
}

void Thread::pop()
{
	if (call_stack.empty())
		return;

	call_stack.back()->thread = nullptr;
	sm->removeFunction(call_stack.back());
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
	event_bitset events = function->getEvents();
	if(events.any())
		functions.push_back(make_pair(events, function));
}

void StateMachine::removeFunction(local_shared_ptr<Function> function)
{
	event_bitset events = function->getEvents();
	if (events.any()) {
		functions.remove(make_pair(events, function));
	}
}

local_shared_ptr<Thread> StateMachine::addThread(local_shared_ptr<Thread> thread)
{
	current_threads.push_back(thread);
	addFunction(thread->getTop());
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

void StateMachine::handleEvent(Event event)
{
	auto _type = make_enum_bitfield(event.eventType);

	for (auto entry : functions) {
		if( (entry.first & _type).any() )
			entry.second->onEvent(event);
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
	handleEvent(event);
}
void StateMachine::onEndDetect(GObject* obj)
{
	Event event(event_type::endDetect, make_any<GObject*>(obj));
	handleEvent(event);
}

void StateMachine::onBulletHit(Bullet* b)
{
	Event event(event_type::bulletHit, make_any<Bullet*>(b));
	handleEvent(event);
}

void StateMachine::onBulletBlock(Bullet* b)
{
	Event event(event_type::bulletBlock, make_any<Bullet*>(b));
	handleEvent(event);
}

void StateMachine::onAlert(Player* p)
{
	Event event(event_type::roomAlert, make_any<Player*>(p));
	handleEvent(event);
}

void StateMachine::onZeroHP()
{
	Event event(event_type::zeroHP, any());
	handleEvent(event);
}

void StateMachine::onZeroStamina()
{
	Event event(event_type::zeroStamina, any());
	handleEvent(event);
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

void StateMachine::addDetectFunction(GType t, detect_function begin, detect_function end)
{
	auto detect = make_local_shared<OnDetectFunction>(this, t, begin, end);
	addFunction(detect);
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
