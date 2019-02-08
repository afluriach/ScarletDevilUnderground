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
#include "FirePattern.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "macros.h"
#include "scenes.h"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"

namespace ai{

//It seems to better to base this on acceleration than force. In some cases mass may
//just be a default value, in which case acceleration makes more sense.
void applyDesiredVelocity(GObject* obj, SpaceVect desired, SpaceFloat acceleration)
{
	SpaceVect floorVel = obj->crntFloorCenterContact.isValid() ? obj->crntFloorCenterContact.get()->getVel() : SpaceVect::zero;
	SpaceFloat traction = obj->getTraction();

	desired += floorVel;
	acceleration *= traction;

    //the desired velocity change
    SpaceVect vv = desired - obj->getVel();
    //the scalar amount of velocity change in one frame
	SpaceFloat dv = acceleration * App::secondsPerFrame;

    //Default case, apply maximum acceleration
    if(square(dv) < vv.lengthSq()){
		SpaceFloat f = obj->getMass() * acceleration;
        SpaceVect ff = f * vv.normalizeSafe();
        obj->applyForceForSingleFrame(ff);
    }
    else{
        obj->setVel(desired);
    }
}

bool isFacingTarget(const GObject* agent, const GObject* target)
{
    SpaceVect targetDirection = (target->getPos() - agent->getPos()).normalize();
    SpaceVect agentFacingVector = agent->getFacingVector();
    bool facing = SpaceVect::dot(agentFacingVector, target->getFacingVector()) < 0;
    bool targetInFrontOfAgent = SpaceVect::dot(agentFacingVector, targetDirection) > 0;

    return facing && targetInFrontOfAgent;
}

bool isFacingTargetsBack(const GObject* agent, const GObject* target)
{
    SpaceVect targetDirection = (target->getPos() - agent->getPos()).normalize();
    SpaceVect agentFacingVector = agent->getFacingVector();

    bool facingBack = SpaceVect::dot(agentFacingVector, target->getFacingVector()) > 0;
    bool targetInFrontOfAgent = SpaceVect::dot(agentFacingVector, targetDirection) > 0;
    
    return facingBack && targetInFrontOfAgent;
}

bool isLineOfSight(const GObject* agent, const GObject* target)
{
    return agent->space->lineOfSight(agent, target);
}

array<SpaceFloat, 4> obstacleFeelerQuad(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 4> results;

	for (int i = 0; i < 4; ++i)
	{
		SpaceVect feeler = dirToVector(static_cast<Direction>(i+1)) * distance;
		results[i] = agent->space->obstacleDistanceFeeler(agent, feeler);
	}

	return results;
}

array<SpaceFloat, 8> obstacleFeeler8(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 8> results;

	for (int i = 0; i < 8; ++i)
	{
		SpaceVect feeler = SpaceVect::ray(distance, i* float_pi / 4.0);
		results[i] = agent->space->obstacleDistanceFeeler(agent, feeler);
	}

	return results;
}

int chooseBestDirection(const array<SpaceFloat, 8>& feelers, SpaceFloat desired_angle, SpaceFloat min_distance)
{
	int bestDirection = -1;
	SpaceFloat bestAngleDistance = float_pi;

	for (int i = 0; i < 8; ++i)
	{
		if (feelers[i] >= min_distance && abs(float_pi * i / 4.0 - desired_angle) < bestAngleDistance)
		{
			bestDirection = i;
			bestAngleDistance = abs(float_pi * i / 4.0 - desired_angle);
		}
	}

	return bestDirection;
}

//Trajectory represents direction/velocity of movement, but scaled to the same length as displacement.
//The difference between these vectors is the distance between the two centers of the objects,
//and thus an indicator of whether a collision will happen, or how the agent needs to move to prevent this.
SpaceVect projectileEvasion(const GObject* bullet, const GObject* agent)
{
	SpaceVect displacementToTarget = agent->getPos() - bullet->getPos();

	SpaceVect trajectoryScaled = bullet->getVel().normalizeSafe() * displacementToTarget.length();

	return trajectoryScaled - displacementToTarget;
}

SpaceVect directionToTarget(const GObject* agent, SpaceVect target)
{
    return (target - agent->getPos()).normalize();
}

SpaceVect displacementToTarget(const GObject* agent, SpaceVect target)
{
    return target - agent->getPos();
}

SpaceFloat distanceToTarget(const GObject* agent, const GObject* target)
{
    return (target->getPos() - agent->getPos()).length();
}

SpaceFloat distanceToTarget(const GObject* agent, SpaceVect target)
{
	return (target - agent->getPos()).length();
}

SpaceFloat viewAngleToTarget(const GObject* agent, const GObject* target)
{
    SpaceVect displacement = target->getPos() - agent->getPos();
    
    if(displacement.lengthSq() > 0.01)
        return canonicalAngle(displacement.toAngle() - agent->getAngle());
    else
        return numeric_limits<SpaceFloat>::infinity();
}

void seek(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration)
{
	SpaceVect displacement = target - agent->getPos();

	if (displacement.lengthSq() < 1e-4)
		return;

    SpaceVect direction = directionToTarget(agent,target);
    
    applyDesiredVelocity(agent, direction*maxSpeed, acceleration);
}

void arrive(GObject* agent, SpaceVect target)
{
	SpaceVect displacement = target - agent->getPos();
	SpaceFloat distance = displacement.length();
	SpaceFloat arrivalTime = distance / agent->getVel().length();
	SpaceFloat accelMag = 2.0 * distance / arrivalTime / arrivalTime;
	SpaceVect direction = displacement.normalizeSafe();

	if (displacement.lengthSq() < 1e-4)
		return;

	applyDesiredVelocity(agent, SpaceVect::zero, accelMag);
}

void flee(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration)
{
	SpaceVect displacement = fleeDirection(agent,target);
    
    applyDesiredVelocity(agent, displacement*maxSpeed, acceleration);
}

SpaceVect fleeDirection(const GObject* agent, SpaceVect target)
{
	SpaceVect displacement = target - agent->getPos();

	if (displacement.lengthSq() < 1e-4)
		return SpaceVect::zero;

	displacement = displacement.normalize();
	displacement = displacement.rotate(float_pi);

	return displacement;
}

void fleeWithObstacleAvoidance(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration)
{
	SpaceVect displacement = fleeDirection(agent, target);
	SpaceFloat distanceMargin = getTurningRadius(agent->getVel().length(), acceleration) + agent->getRadius();

	if (agent->space->obstacleDistanceFeeler(agent, displacement * distanceMargin) < distanceMargin)
	{
		//Choose an alternate direction to move.

		array<SpaceFloat, 8> feelers = obstacleFeeler8(agent, distanceMargin);

		int directionID = chooseBestDirection(feelers, displacement.toAngle(), distanceMargin);

		if (directionID == -1) {
			applyDesiredVelocity(agent, SpaceVect::zero, acceleration);
		}
		else {
			applyDesiredVelocity(agent, SpaceVect::ray(maxSpeed, directionID* float_pi / 4.0), acceleration);
		}
	}
	else
	{
		flee(agent, target, maxSpeed, acceleration);
	}
}


SpaceFloat getStoppingTime(SpaceFloat speed, SpaceFloat acceleration)
{
	return speed / acceleration;
}

SpaceFloat getStoppingDistance(SpaceFloat speed, SpaceFloat acceleration)
{
	SpaceFloat t = getStoppingTime(speed, acceleration);

	return 0.5 * acceleration * t * t;
}

SpaceFloat getTurningRadius(SpaceFloat speed, SpaceFloat acceleration)
{
	return speed * speed / acceleration;
}


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

void StateMachine::addDetectFunction(GType t, detect_function f)
{
	detectHandlers.insert_or_assign(t, f);
}

void StateMachine::addEndDetectFunction(GType t, detect_function f)
{
	endDetectHandlers.insert_or_assign(t, f);
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

Agent* StateMachine::getAgent() {
	return dynamic_cast<Agent*>(agent);
}

unsigned int StateMachine::getFrame() {
	return frame;
}

Thread* StateMachine::getCrntThread() {
	return crntThread;
}

Seek::Seek(GSpace* space, const ValueMap& args) {
    target = getObjRefFromStringField(space, args, "target_name");
}

Seek::Seek(GObject* target) : target(target)
{}

void Seek::update(StateMachine& sm)
{
	if (target.isValid()) {
		ai::seek(
			sm.agent,
			target.get()->getPos(),
			sm.agent->getMaxSpeed(),
			sm.agent->getMaxAcceleration()
		);
		sm.agent->setDirection(toDirection(
            ai::directionToTarget(
                sm.agent,
                target.get()->getPos()
            )
        ));
	}
	else{
		sm.getCrntThread()->pop();
    }
}

MaintainDistance::MaintainDistance(gobject_ref target, SpaceFloat distance, SpaceFloat margin) :
target(target),
distance(distance),
margin(margin)
{}

MaintainDistance::MaintainDistance(GSpace* space, const ValueMap& args)
{
    target = getObjRefFromStringField(space, args, "target");
    distance = getFloat(args, "distance");
    margin = getFloat(args, "margin");
}

void MaintainDistance::update(StateMachine& sm)
{
	if (target.get()) {
        SpaceFloat crnt_distance = distanceToTarget(sm.agent,target.get());
		SpaceFloat stop_dist = getStoppingDistance(sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
    
		if (abs(crnt_distance - distance) < stop_dist) {
			ai::arrive(
				sm.agent,
				SpaceVect::ray(distance, float_pi + directionToTarget(sm.agent, target.get()->getPos()).toAngle())
			);
		}

        else if(crnt_distance > distance + margin){
            ai::seek(
                sm.agent,
                target.get()->getPos(),
                sm.agent->getMaxSpeed(),
                sm.agent->getMaxAcceleration()
            );
        }
        else if(crnt_distance < distance + margin){
            ai::fleeWithObstacleAvoidance(
                sm.agent,
                target.get()->getPos(),
                sm.agent->getMaxSpeed(),
                sm.agent->getMaxAcceleration()
            );
        }
	}
	else {
		ai::applyDesiredVelocity(sm.agent, SpaceVect::zero, sm.agent->getMaxAcceleration());
	}
}

OccupyMidpoint::OccupyMidpoint(gobject_ref target1, gobject_ref target2) :
target1(target1),
target2(target2)
{
}

void OccupyMidpoint::update(StateMachine& sm)
{
	GObject* t1 = target1.get();
	GObject* t2 = target2.get();

	if (!t1 || !t2) {
		sm.pop();
		return;
	}

	SpaceVect midpoint = (t1->getPos() + t2->getPos()) / 2.0;
	SpaceFloat crnt_distance = distanceToTarget(sm.agent, midpoint);
	SpaceFloat stop_dist = getStoppingDistance(sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());

	if (crnt_distance > stop_dist) {
		seek(sm.agent, midpoint, sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
	}
	else {
		arrive(sm.agent, midpoint);
	}
}

Scurry::Scurry(GSpace* space, GObject* _target, SpaceFloat _distance, SpaceFloat length) :
distance(_distance),
target(_target)
{
	startFrame = space->getFrame();

	if (length > 0.0)
		endFrame = startFrame + App::framesPerSecond*length;
	else
		endFrame = 0;
}

void Scurry::update(StateMachine& sm)
{
	if (!target.isValid() || endFrame != 0 && sm.agent->space->getFrame() >= endFrame) {
		sm.pop();
	}

	SpaceVect displacement = displacementToTarget(sm.agent, target.get()->getPos());

	SpaceFloat angle = displacement.toAngle();
	if (!scurryLeft) {
		angle += float_pi;
	}
	scurryLeft = !scurryLeft;

	array<SpaceFloat, 8> obstacleFeelers = obstacleFeeler8(sm.agent, distance);
	int direction = chooseBestDirection(obstacleFeelers, angle, distance);

	if (direction != -1) {
		sm.push(make_shared <MoveToPoint>(
			sm.agent->getPos() + SpaceVect::ray(distance, direction * float_pi / 4.0)
		));
	}
}

Flee::Flee(GObject* target, SpaceFloat distance) :
	target(target),
	distance(distance)
{}


Flee::Flee(GSpace* space, const ValueMap& args) {
    if(args.find("target_name") == args.end()){
        log("Seek::Seek: target_name missing.");
    }
    target = space->getObject(args.at("target_name").asString());
    
    if(!target.isValid()){
        log("Flee::Flee: target object %s not found.", args.at("target_name").asString().c_str() );
    }
    
    if(args.find("flee_distance") == args.end()){
        log("Flee::Flee: flee_distance missing.");
		distance = 0.0;
    }
    else{
        distance = args.at("flee_distance").asFloat();
    }
}

void Flee::update(StateMachine& sm)
{
	if (target.isValid()) {
		ai::fleeWithObstacleAvoidance(
			sm.agent,
			target.get()->getPos(),
			sm.agent->getMaxSpeed(),
			sm.agent->getMaxAcceleration()
		);
		sm.agent->setDirection(toDirection(
            ai::directionToTarget(
                sm.agent,
                target.get()->getPos()
            )
        ));
	}
	else{
        sm.getCrntThread()->pop();
    }

}

EvadePlayerProjectiles::EvadePlayerProjectiles() {}

EvadePlayerProjectiles::EvadePlayerProjectiles(GSpace* space, const ValueMap& args) {}

void EvadePlayerProjectiles::update(StateMachine& sm)
{
	list<GObject*> objs = sm.getAgent()->getSensedObjects();
	
	GObject* closest = nullptr;
	SpaceFloat closestDistance = numeric_limits<SpaceFloat>::infinity();
	 
	for(GObject* obj: objs)
	{
		if (obj->getType() != GType::playerBullet)
			continue;

		SpaceFloat crntDist = distanceToTarget(obj, sm.agent);

		if (crntDist < closestDistance) {
			closestDistance = crntDist;
			closest = obj;
		}
	}

	if (closest != nullptr)
	{
		SpaceVect offset = projectileEvasion(closest, sm.agent);

		if(offset.length() > closest->getRadius() + sm.agent->getRadius() )
		{
			sm.agent->setVel(SpaceVect::zero);
		}
		else
		{
			applyDesiredVelocity(sm.agent, offset.normalize()*-1.0f * sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
		}
	}
}

IdleWait::IdleWait(GSpace* space, const ValueMap& args)
{
    auto it = args.find("waitTime");
    
    if(it == args.end()){
        log("IdleWait::IdleWait: waitTime missing from ValueMap!");
        remaining = 0;
        return;
    }
    
    if(!it->second.isNumber()){
        log("IdleWait::IdleWait: waitTime is not a number!");
        remaining = 0;
        return;
    }

    if(it->second.asFloat() < 0.0f){
        log("IdleWait::IdleWait: waitTime is negative!");
        remaining = 0;
        return;
    }
    
    SpaceFloat waitSeconds = getFloat(args, "waitTime");
    remaining = App::framesPerSecond * waitSeconds;
}

IdleWait::IdleWait(unsigned int frames) :
	remaining(frames)
{}


void IdleWait::update(StateMachine& fsm)
{
	if (remaining == 0)
		fsm.getCrntThread()->pop();
	--remaining;

	ai::applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.agent->getMaxAcceleration());
}

LookAround::LookAround(SpaceFloat angularVelocity) :
angularVelocity(angularVelocity)
{
}

void LookAround::update(StateMachine& fsm)
{
	fsm.agent->rotate(angularVelocity * App::secondsPerFrame);
}

CircleAround::CircleAround(SpaceVect center, SpaceFloat startingAngularPos, SpaceFloat angularSpeed) :
center(center),
angularPosition(startingAngularPos),
angularSpeed(angularSpeed)
{
}

void CircleAround::init(StateMachine& fsm)
{
}

void CircleAround::update(StateMachine& fsm)
{
	SpaceFloat radius = distanceToTarget(fsm.agent, center);
	SpaceFloat angleDelta = angularSpeed * App::secondsPerFrame;

	angularPosition += angleDelta;

	SpaceVect agentPos = center + SpaceVect::ray(radius, angularPosition);

	fsm.agent->setPos(agentPos);
	fsm.agent->setAngle(angularPosition);
}

Flank::Flank(gobject_ref target) :
target(target)
{
}

void Flank::init(StateMachine& fsm)
{
}

void Flank::update(StateMachine& fsm)
{
	if (!target.isValid()) {
		fsm.pop();
	}

	SpaceVect pos = target.get()->getPos();
	SpaceFloat angle = target.get()->getAngle();
	SpaceFloat this_angle = viewAngleToTarget(target.get(), fsm.agent);

	if (abs(this_angle) < float_pi / 4.0) {
		//move to side flank

		if (this_angle < 0) {
			fsm.push(make_shared<MoveToPoint>(
				SpaceVect::ray(1.0, angle - float_pi / 2.0) + pos
			));
		}
		else {
			fsm.push(make_shared<MoveToPoint>(
				SpaceVect::ray(1.0, angle + float_pi / 2.0) + pos
			));
		}
	}
	else
	{
		//move to rear flank
		fsm.push(make_shared<MoveToPoint>(
			SpaceVect::ray(1.0, angle + float_pi ) + pos
		));
	}
}

QuadDirectionLookAround::QuadDirectionLookAround(boost::rational<int> secondsPerDirection, bool clockwise) :
secondsPerDirection(secondsPerDirection),
timeRemaining(secondsPerDirection),
clockwise(clockwise)
{

}

void QuadDirectionLookAround::update(StateMachine& fsm)
{
	timerDecrement(timeRemaining);

	if (timeRemaining <= 0) {
		fsm.agent->rotate(float_pi / 2.0 * (clockwise ? 1.0 : -1.0));
		timeRemaining = secondsPerDirection;
	}
}

AimAtTarget::AimAtTarget(gobject_ref target) :
target(target)
{
}

void AimAtTarget::update(StateMachine& fsm)
{
	if (!target.isValid())
		return;

	fsm.agent->setAngle(directionToTarget(fsm.agent, target.get()->getPos()).toAngle());
}

MoveToPoint::MoveToPoint(GSpace* space, const ValueMap& args)
{
    auto xIter = args.find("target_x");
    auto yIter = args.find("target_y");
    
    if(xIter == args.end()){
        log("MoveToPoint::MoveToPoint: target_x missing from ValueMap");
        return;
    }
    if(yIter == args.end()){
        log("MoveToPoint::MoveToPoint: target_y missing from ValueMap");
        return;
    }
    
    const Value &x = xIter->second;
    const Value &y = yIter->second;

//Cocos2D ValueMap does not correctly read data type.
//    if(!x.isNumber()){
//        log("MoveToPoint::MoveToPoint: target_x is not a number.");
//        return;
//    }
//    if(!y.isNumber()){
//        log("MoveToPoint::MoveToPoint: target_y is not a number.");
//        return;
//    }
    
    target  = SpaceVect(x.asFloat(), y.asFloat());
}

MoveToPoint::MoveToPoint(SpaceVect target) :
	target(target)
{}

void MoveToPoint::update(StateMachine& fsm)
{
    SpaceFloat dist2 = (fsm.agent->getPos() - target).lengthSq();
	SpaceFloat stoppingDist2 = getStoppingDistance(fsm.agent->getVel().length(), fsm.agent->getMaxAcceleration());

	if (dist2 < 0.0625) {
		fsm.pop();
	}
	else if (dist2 <= stoppingDist2) {
		arrive(fsm.agent, target);
	}
	else {
		seek(fsm.agent, target, fsm.agent->getMaxSpeed(), fsm.agent->getMaxAcceleration());
	}    
}

shared_ptr<FollowPath> FollowPath::pathToTarget(GSpace* space, gobject_ref agent, gobject_ref target)
{
	if (!agent.isValid() || !target.isValid()) {
		return nullptr;
	}

	return make_shared<ai::FollowPath>(
		space->pathToTile(
			toIntVector(agent.get()->getPos()), toIntVector(target.get()->getPos())
		),
		false
	);
}


FollowPath::FollowPath(Path path, bool loop) :
	path(path),
	loop(loop)
{}

FollowPath::FollowPath(GSpace* space, const ValueMap& args)
{
	auto name_it = args.find("pathName");
	auto loop_it = args.find("loop");

	if (name_it == args.end()) {
		log("FollowPath: pathName not provided!");
	}

	Path const* p = space->getPath(name_it->second.asString());

	if (!p) {
		log("FollowPath: pathName %s not found!", name_it->second.asString().c_str());
	}
	else {
		path = *p;
	}

	if (loop_it != args.end() && boost::iequals(loop_it->second.asString(), "true")) {
		loop = true;
	}
}

void FollowPath::update(StateMachine&  fsm)
{
	if (currentTarget < path.size()) {
		fsm.agent->setDirection(toDirection(ai::directionToTarget(fsm.agent, path[currentTarget])));
		fsm.push(make_shared<MoveToPoint>(path[currentTarget]));
		++currentTarget;
	}
	else if (loop && path.size() > 0) {
		currentTarget = 0;
	}
	else {
		fsm.pop();
	}
}

Wander::Wander(GSpace* space, const ValueMap& args) :
    init_float_field(minWait,1.0f),
    init_float_field(maxWait,1.0f),
    init_float_field(minDist,1.0f),
    init_float_field(maxDist,1.0f)
{}

Wander::Wander() : minWait(1.0), maxWait(3.0), minDist(2.0), maxDist(4.0)
{}

void Wander::update(StateMachine& fsm)
{
    SpaceFloat dist = App::getRandomFloat(minDist, maxDist);
    vector<Direction> directions;
    SpaceVect target;
    
    enum_foreach(Direction,d,right,end)
    {
        target = dirToVector(d)*dist;
        if(!fsm.agent->space->obstacleFeeler(fsm.agent, target)){
            directions.push_back(d);
        }
    }

    if(directions.empty())
        return;

    int randomIdx = App::getRandomInt(0, directions.size()-1);
    
    fsm.push(make_shared<MoveToPoint>(fsm.agent->getPos()+dirToVector(directions[randomIdx])*dist));
    
    fsm.push(make_shared<Operation>(
        [=](StateMachine& sm) -> void {
            sm.agent->setDirection(directions[randomIdx]);
    }));
    
    int waitFrames = App::getRandomInt(minWait*App::framesPerSecond, maxWait*App::framesPerSecond);
    fsm.push(make_shared<IdleWait>(waitFrames));
}

FireAtTarget::FireAtTarget(shared_ptr<FirePattern> pattern, gobject_ref target) :
	pattern(pattern),
	target(target)
{}

void FireAtTarget::update(StateMachine& sm)
{
	if (!target.isValid()) {
		sm.pop();
	}

	sm.agent->setAngle(
		directionToTarget(sm.agent, target.get()->getPos()).toAngle()
	);

	pattern.get()->update();

	if (pattern.get()->fireIfPossible()) {
		App::playSoundSpatial("sfx/shot.wav", sm.agent->getPos(), sm.agent->getVel());
	}
}

FireIfTargetVisible::FireIfTargetVisible(shared_ptr<FirePattern> pattern, gobject_ref target) :
	pattern(pattern),
	target(target)
{}

void FireIfTargetVisible::update(StateMachine& sm)
{
	RadarObject* ro = dynamic_cast<RadarObject*>(sm.agent);

	if (!ro || !target.isValid()) {
		sm.pop();
		return;
	}
	
	pattern.get()->update();

	if (ro->isObjectVisible(target.get()) && sm.agent->space->isInPlayerRoom(sm.agent->getPos()))
	{
		if (pattern.get()->fireIfPossible()) {
			App::playSoundSpatial("sfx/shot.wav", sm.agent->getPos(), sm.agent->getVel());
		}
	}
}

Operation::Operation(std::function<void(StateMachine&)> op) :
	op(op)
{}

void Operation::update(StateMachine& sm) {
	op(sm);
	sm.pop();
}

Cast::Cast(SpellGeneratorType spell_generator) :
spell_generator(spell_generator)
{
}

void Cast::onEnter(StateMachine& sm)
{
	sm.agent->cast(spell_generator(sm.agent));
}

void Cast::update(StateMachine& sm)
{
	if (!sm.agent->isSpellActive())
		sm.pop();
}

void Cast::onExit(StateMachine& sm)
{
	sm.agent->stopSpell();
}

HPCast::HPCast(SpellGeneratorType spell_generator, float hp_difference) :
	spell_generator(spell_generator),
	hp_difference(hp_difference)
{
}

void HPCast::onEnter(StateMachine& sm)
{
	caster_starting = sm.getAgent()->getHealth();
	sm.agent->cast(spell_generator(sm.agent));
}

void HPCast::update(StateMachine& sm)
{
	if (sm.getAgent()->getHealth() < (caster_starting - hp_difference)) {
		sm.agent->stopSpell();
	}

	if (!sm.agent->isSpellActive()) {
		sm.pop();
	}
}

void HPCast::onExit(StateMachine& sm)
{
	sm.agent->stopSpell();
}

}//end NS
