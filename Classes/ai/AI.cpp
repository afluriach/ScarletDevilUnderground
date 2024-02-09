//
//  AI.cpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "AreaSensor.hpp"
#include "Bomb.hpp"
#include "Bullet.hpp"
#include "FirePattern.hpp"
#include "LuaAPI.hpp"
#include "object_properties.hpp"
#include "Player.hpp"
#include "sol_util.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"
#include "value_map.hpp"

namespace ai{

update_return update_return::makePush(local_shared_ptr<Function> f)
{
	return update_return(0, 0.0f, f);
}

update_return update_return::makePop()
{
	return update_return(-1, 0.0f, nullptr);
}

update_return update_return::makeSteady(float update)
{
	return update_return(0, update, nullptr);
}

update_return::update_return() :
	update_return(0, 0.0f, nullptr)
{
}

update_return::update_return(int idx, float update, local_shared_ptr<Function> f) :
	idx(idx),
	update(update),
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

GSpace* Function::getSpace() const {
	return fsm->getSpace();
}

GObject* Function::getObject() const {
	return fsm->getObject();
}

Agent* Function::getAgent() const {
	return fsm->getAgent();
}

Player* Function::getPlayer() const{
    return fsm->getPlayer();
}

physics_context* Function::getPhys() const {
	return getSpace()->physicsContext.get();
}

bool Function::castSpell(const SpellDesc* desc)
{
	if (isSpellActive())
		stopSpell();

	spell = getObject()->cast(desc);
	return spell;
}

local_shared_ptr<Spell> Function::castSpellManual(const SpellDesc* desc)
{
	return getObject()->cast(desc);
}

bool Function::isSpellActive()
{
	return spell && spell->isSpellActive();
}

void Function::stopSpell()
{
	if (spell) {
		getSpace()->spellSystem->stopSpell(spell);
		spell.reset();
	}
}

Thread::Thread(StateMachine* sm) :
	sm(sm)
{
}

Thread::~Thread()
{
}

void Thread::update()
{
	Function* crnt = !call_stack.empty() ? call_stack.back().get() : nullptr;
	if (!crnt) return;

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
	call_stack.push_back(newState);
	newState->onEnter();
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

void Thread::clear()
{
	while (call_stack.size() > 0) {
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

StateMachine::StateMachine(GObject *const agent, local_shared_ptr<ai_properties> props) :
props(props),
agent(agent)
{
    frame = getSpace()->getFrame();
	_stack = make_local_shared<Thread>(this);
}

StateMachine::~StateMachine()
{
}

void StateMachine::update()
{
	_stack->update();
}

void StateMachine::pushFunction(local_shared_ptr<Function> function)
{
	if (!function) return;
	_stack->push(function);
}

void StateMachine::setFunction(local_shared_ptr<Function> function)
{
    _stack->clear();
    _stack->push(function);
}

void StateMachine::onDetectEnemy(Agent* enemy)
{
	callInterface<Agent*>("detectEnemy", &ai::Function::detectEnemy, enemy);
}

void StateMachine::onEndDetectEnemy(Agent* enemy)
{
	callInterface<Agent*>("endDetectEnemy", &ai::Function::endDetectEnemy, enemy);
}

void StateMachine::onDetectBomb(Bomb* bomb)
{
	callInterface<Bomb*>("detectBomb", &ai::Function::detectBomb, bomb);
}

void StateMachine::onDetectBullet(Bullet* bullet)
{
	callInterface<Bullet*>("detectBullet", &ai::Function::detectBullet, bullet);
}

void StateMachine::onBulletHit(Bullet* b)
{
	callInterface<Bullet*>("bulletHit", &ai::Function::bulletHit, b);
}

void StateMachine::onBulletBlock(Bullet* b)
{
	callInterface<Bullet*>("bulletBlock", &ai::Function::bulletBlock, b);
}

void StateMachine::enemyRoomAlert(Agent* enemy)
{
	callInterface<Agent*>("enemyRoomAlert", &ai::Function::enemyRoomAlert, enemy);
}

void StateMachine::onZeroHP()
{
	callInterface("zeroHP", &ai::Function::zeroHP);
}

void StateMachine::onZeroStamina()
{
	callInterface("zeroStamina", &ai::Function::zeroStamina);
}

string StateMachine::toString()
{
    stringstream ss;
    
    ss << "StateMachine for " << agent->getName() << ":\n";
    
    Thread* t = _stack.get();
    ss << "thread " << t << ", stack:  " << t->getStack() << "\n";

	return ss.str();
}

GSpace* StateMachine::getSpace() {
	return agent->space;
}

Agent* StateMachine::getAgent() {
	return dynamic_cast<Agent*>(agent);
}

Player* StateMachine::getPlayer() {
	return dynamic_cast<Player*>(agent);
}

RoomSensor* StateMachine::getRoomSensor() {
	return dynamic_cast<RoomSensor*>(agent);
}

unsigned int StateMachine::getFrame() {
	return frame;
}

}//end NS
