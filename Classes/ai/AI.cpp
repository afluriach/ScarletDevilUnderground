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
#include "Bullet.hpp"
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
		results[i] = agent->space->obstacleDistanceFeeler(agent, feeler, agent->getRadius()*2.0);
	}

	return results;
}

array<SpaceFloat, 8> obstacleFeeler8(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 8> results;

	for (int i = 0; i < 8; ++i)
	{
		SpaceVect feeler = SpaceVect::ray(distance, i* float_pi / 4.0);
		results[i] = agent->space->obstacleDistanceFeeler(agent, feeler, agent->getRadius()*2.0);
	}

	return results;
}

array<SpaceFloat, 8> wallFeeler8(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 8> results;

	for (int i = 0; i < 8; ++i)
	{
		SpaceVect feeler = SpaceVect::ray(distance, i* float_pi / 4.0);
		results[i] = agent->space->wallDistanceFeeler(agent, feeler);
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

bool isInFieldOfView(GObject* agent, SpaceVect target, SpaceFloat fovAngleScalarProduct)
{
	SpaceVect facingUnit = SpaceVect::ray(1.0, agent->getAngle());
	SpaceVect displacementUnit = (target - agent->getPos()).normalize();
	SpaceFloat scalar = SpaceVect::dot(facingUnit, displacementUnit);

	return scalar >= fovAngleScalarProduct;
}

bool isObstacle(Agent* agent, SpaceVect target)
{
	SpaceFloat maxSpeed = agent->getMaxSpeed();
	SpaceFloat acceleration = agent->getMaxAcceleration();
	SpaceVect displacement = compute_seek(agent, target).normalizeSafe();
	SpaceFloat distanceMargin = getTurningRadius(agent->getVel().length(), acceleration) + agent->getRadius();

	return (agent->space->obstacleDistanceFeeler(agent, displacement * distanceMargin, agent->getRadius()*2.0) < distanceMargin);
}

SpaceVect compute_seek(Agent* agent, SpaceVect target)
{
	SpaceVect displacement = target - agent->getPos();

	if (displacement.lengthSq() < 1e-4)
		return SpaceVect::zero;

	SpaceVect direction = directionToTarget(agent, target);

	SpaceVect v = direction*agent->getMaxSpeed();
	return (v - agent->getVel()).limit(agent->getMaxAcceleration());
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

bool moveToPoint(GObject* agent, SpaceVect target, SpaceFloat arrivalMargin, bool stopForObstacle)
{
	SpaceFloat _accel = agent->getMaxAcceleration();
	SpaceFloat dist2 = (agent->getPos() - target).lengthSq();
	SpaceFloat stoppingDist = getStoppingDistance(agent->getVel().length(), _accel);
	SpaceFloat radius = agent->getRadius();
	SpaceFloat angle = agent->getAngle();

	if (stopForObstacle && agent->space->obstacleFeeler(agent, SpaceVect::ray(stoppingDist+radius, angle), radius*2.0)) {
		applyDesiredVelocity(agent, SpaceVect::zero, _accel);
		return false;
	}
	else if (dist2 < arrivalMargin*arrivalMargin) {
		return true;
	}
	else if (dist2 <= stoppingDist*stoppingDist) {
		arrive(agent, target);
		return false;
	}
	else {
		seek(agent, target, agent->getMaxSpeed(), _accel);
		return false;
	}
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

	if (agent->space->obstacleDistanceFeeler(agent, displacement * distanceMargin, agent->getRadius()*2.0) < distanceMargin)
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

SpaceVect bezier(array<SpaceVect, 3> points, SpaceFloat t)
{
	SpaceFloat u = 1.0 - t;
	return u * u*points[0] + 2.0*u*t*points[1] + t * t*points[2];
}

SpaceVect bezierAcceleration(array<SpaceVect, 3> points)
{
	return 2.0 * (points[2] - 2.0*points[1] - points[0]);
}

bullet_collide_function buildStressFromHits(float hpStressScale)
{
	return [hpStressScale](StateMachine& sm, Bullet* b)->void {
		Agent* a = sm.getAgent();
		float hp = b->getAttributeEffect().at(Attribute::hp);
		a->modifyAttribute(Attribute::stress, -1.0f * hp * hpStressScale);
	};
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

void StateMachine::onBulletHit(Bullet* b)
{
	if (bulletHandler) {
		bulletHandler(*this, b);
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

void StateMachine::removeDetectFunction(GType t)
{
	detectHandlers.erase(t);
}

void StateMachine::removeEndDetectFunction(GType t)
{
	endDetectHandlers.erase(t);
}

void StateMachine::setBulletHitFunction(bullet_collide_function f)
{
	bulletHandler = f;
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

Seek::Seek(GSpace* space, const ValueMap& args) {
    target = getObjRefFromStringField(space, args, "target_name");
	usePathfinding = getBoolOrDefault(args, "use_pathfinding", false);
}

Seek::Seek(GObject* target, bool usePathfinding) :
	target(target),
	usePathfinding(usePathfinding)
{}

void Seek::update(StateMachine& sm)
{
	if (target.isValid()) {
		if (usePathfinding && isObstacle(sm.getAgent(), target.get()->getPos())) {
			sm.push(ai::PathToTarget::create(sm.agent, target.get()));
		}
		else {
			seek(
				sm.agent,
				target.get()->getPos(),
				sm.agent->getMaxSpeed(),
				sm.agent->getMaxAcceleration()
			);
		}
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

const SpaceFloat Flock::separationDesired = 5.0;

Flock::Flock()
{
}

void Flock::update(StateMachine& sm)
{
	SpaceVect _separate = separate(sm.getAgent()) * 1.5;
	SpaceVect _align = align(sm.getAgent()) * 0.75;
	SpaceVect _cohesion = cohesion(sm.getAgent()) * 0.75;

	SpaceVect sum = _separate + _align + _cohesion;

	sm.agent->applyForceForSingleFrame(sum.setMag(sm.getAgent()->getMaxAcceleration()) );
}

void Flock::onDetectNeighbor(Agent* agent)
{
	neighbors.insert(agent);
}

void Flock::endDetectNeighbor(Agent* agent)
{
	neighbors.erase(agent);
}

SpaceVect Flock::separate(Agent* _agent)
{
	SpaceVect steer_acc;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();
		SpaceVect disp = _agent->getPos() - other->getPos();

		//Actual magnitude added from interaction is 1/x.
		if (disp.lengthSq() < separationDesired*separationDesired) {
			steer_acc += disp.normalizeSafe() / disp.length();
			++count;
		}
	}

	if (count > 0) {
		steer_acc /= count;
		steer_acc = steer_acc.normalize() * _agent->getMaxSpeed();
		steer_acc -= _agent->getVel();
		steer_acc.limit(_agent->getMaxAcceleration());

	}

	array<SpaceFloat, 8> walls = ai::wallFeeler8(_agent, separationDesired);

	for (size_t i = 0; i < 8; ++i) {
		if (walls[i] < separationDesired) {
			SpaceVect v = SpaceVect::ray(1.0 / walls[i], i * float_pi / 4.0).rotate(float_pi);

			steer_acc += v;
			++count;
		}
	}

	return steer_acc;
}

//Calculate average velocity of neighbors
SpaceVect Flock::align(Agent* _agent)
{
	SpaceVect sum;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();

		sum += other->getVel();
		++count;
	}

	if (count > 0) {
		sum /= count;

		SpaceVect vel = sum.normalizeSafe() * _agent->getMaxSpeed();
		SpaceVect steer = vel - _agent->getVel();

		return steer.limit(_agent->getMaxAcceleration());
	}
	else {
		return SpaceVect::zero;
	}
}

//Calculate average position of neighbors;
SpaceVect Flock::cohesion(Agent* _agent)
{
	SpaceVect sum;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();

		sum += other->getPos();
		++count;
	}

	if (count > 0) {
		return compute_seek(_agent, sum / count);
	}
	else {
		return SpaceVect::zero;
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
	list<GObject*> objs = sm.getAgent()->getSensedObjectsByGtype(GType::playerBullet);
	
	GObject* closest = nullptr;
	SpaceFloat closestDistance = numeric_limits<SpaceFloat>::infinity();
	 
	for(GObject* obj: objs)
	{
		SpaceFloat crntDist = distanceToTarget(obj, sm.agent);

		if (crntDist < closestDistance) {
			closestDistance = crntDist;
			closest = obj;
		}
	}

	active = closest != nullptr;

	if (closest != nullptr)
	{
		SpaceVect offset = projectileEvasion(closest, sm.agent);
		if(!offset.isZero())
			applyDesiredVelocity(sm.agent, offset.normalize()*-1.0f * sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
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

LookTowardsFire::LookTowardsFire()
{
}

void LookTowardsFire::onEnter(StateMachine& fsm)
{
	fsm.setBulletHitFunction(bind(&LookTowardsFire::onBulletCollide, this, placeholders::_1, placeholders::_2));
}

void LookTowardsFire::update(StateMachine& fsm)
{
	hitAccumulator -= (looking*lookTimeCoeff + (1-looking)*timeCoeff)* App::secondsPerFrame;
	hitAccumulator = max(hitAccumulator, 0.0f);

	if (hitAccumulator == 0.0f) {
		directionAccumulator = SpaceVect::zero;
		looking = false;
	}
	else if (!looking && hitAccumulator >= 1.0f) {
		fsm.agent->setAngle(directionAccumulator.toAngle());
		looking = true;
	}

	if (looking) {
		applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.getAgent()->getMaxAcceleration());
	}
}

void LookTowardsFire::onExit(StateMachine& fsm)
{
	//should remove the collision handler function here
}

void LookTowardsFire::onBulletCollide(StateMachine& fsm, Bullet* b)
{
	SpaceVect bulletDirection = b->getVel().normalize().rotate(float_pi);
	hitAccumulator += hitCost;
	directionAccumulator += bulletDirection;

	if (looking) {
		fsm.agent->setAngle(bulletDirection.toAngle());
	}
}

bitset<lockCount> LookTowardsFire::getLockMask()
{
	return looking ?
		make_enum_bitfield(ResourceLock::movement) | make_enum_bitfield(ResourceLock::look) :
		bitset<lockCount>()
	;
}

const double MoveToPoint::arrivalMargin = 0.125;

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
	bool arrived = moveToPoint(fsm.agent, target, arrivalMargin, false);
	
	if (arrived) {
		fsm.pop();
	}
}

BezierMove::BezierMove(array<SpaceVect, 3> points, SpaceFloat rate) :
	points(points),
	rate(rate)
{
}

void BezierMove::update(StateMachine& fsm)
{
	fsm.agent->setPos(bezier(points, t));

	timerIncrement(t, rate);
	if (t >= 1.0) {
		fsm.agent->setPos(bezier(points, 1.0));
		fsm.pop();
	}
}

shared_ptr<FollowPath> FollowPath::pathToTarget(GSpace* space, gobject_ref agent, gobject_ref target)
{
	if (!agent.isValid() || !target.isValid()) {
		return nullptr;
	}

	return make_shared<ai::FollowPath>(
		space->pathToTile(
			toIntVector(agent.get()->getPos()),
			toIntVector(target.get()->getPos())
		),
		false,
		false
	);
}

FollowPath::FollowPath(Path path, bool loop, bool stopForObstacle) :
	path(path),
	loop(loop),
	stopForObstacle(stopForObstacle)
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
		bool arrived = moveToPoint(fsm.agent, path[currentTarget], MoveToPoint::arrivalMargin, stopForObstacle);
		currentTarget += arrived;
	}
	else if (loop && path.size() > 0) {
		currentTarget = 0;
	}
	else {
		fsm.pop();
	}
}

shared_ptr<PathToTarget> PathToTarget::create(GObject* agent, GObject* target)
{
	Path p = agent->space->pathToTile(
		toIntVector(agent->getPos()),
		toIntVector(target->getPos())
	);

	return make_shared<PathToTarget>(p, target);
}

PathToTarget::PathToTarget(Path path, gobject_ref target) :
	FollowPath(path, false, false),
	target(target)
{
}

void PathToTarget::update(StateMachine& fsm)
{
	if (ai::isLineOfSight(fsm.agent, target.get())) {
		fsm.pop();
	}
	else {
		FollowPath::update(fsm);
	}
}

Wander::Wander(GSpace* space, const ValueMap& args) :
    init_float_field(minWait,1.0f),
    init_float_field(maxWait,1.0f),
    init_float_field(minDist,1.0f),
    init_float_field(maxDist,1.0f)
{}

Wander::Wander(SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist) :
	minWait(minWait),
	maxWait(maxWait),
	minDist(minDist),
	maxDist(maxDist)
{
}

Wander::Wander(SpaceFloat waitInterval, SpaceFloat moveDist) :
	Wander(waitInterval, waitInterval, moveDist, moveDist)
{}

Wander::Wander() : 
	Wander(1.0, 1.0, 1.0, 1.0)
{}

pair<Direction, SpaceFloat> Wander::chooseMovement(StateMachine& fsm)
{
	array<SpaceFloat, 4> feelers = ai::obstacleFeelerQuad(fsm.agent, maxDist);
	vector<Direction> directions;
	directions.reserve(4);

	//First, select a direction that allows maximum movement, if possible.
	enum_foreach(Direction, d, right, end)
	{
		if (feelers[to_int(d) - 1] == maxDist) {
			directions.push_back(d);
		}
	}

	if (!directions.empty()) {
		return make_pair(directions.at(App::getRandomInt(0, directions.size()-1)), maxDist);
	}

	//Select a direction that allows at least minimum desired distance.
	enum_foreach(Direction, d, right, end)
	{
		if (feelers[to_int(d) - 1] >= minDist) {
			directions.push_back(d);
		}
	}

	if (!directions.empty()) {
		int idx = App::getRandomInt(0, directions.size() - 1);
		return make_pair(directions.at(idx), feelers[idx]);
	}

	return make_pair(Direction::none, 0.0);
}

void Wander::update(StateMachine& fsm)
{
	timerDecrement(waitTimer);

	if (waitTimer <= 0.0) {
		pair<Direction, SpaceFloat> movement = chooseMovement(fsm);

		if (movement.first != Direction::none && movement.second > 0.0) {
			fsm.agent->setDirection(movement.first);
			fsm.push(make_shared<MoveToPoint>(
				fsm.agent->getPos() + dirToVector(movement.first)*movement.second
			));
			waitTimer = App::getRandomFloat(minWait, maxWait);
		}
	}
	else {
		applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.agent->getMaxAcceleration());
	}
}

FireAtTarget::FireAtTarget(gobject_ref target) :
	target(target)
{}

void FireAtTarget::update(StateMachine& sm)
{
	FirePattern* fp = sm.getAgent()->getFirePattern();
	if (!target.isValid() || !fp) {
		sm.pop();
		return;
	}

	sm.agent->setAngle(
		directionToTarget(sm.agent, target.get()->getPos()).toAngle()
	);

	if (fp->fireIfPossible()) {
		App::playSoundSpatial("sfx/shot.wav", sm.agent->getPos(), sm.agent->getVel());
	}
}

FireIfTargetVisible::FireIfTargetVisible(gobject_ref target) :
	target(target)
{}

void FireIfTargetVisible::update(StateMachine& sm)
{
	RadarObject* ro = dynamic_cast<RadarObject*>(sm.agent);
	FirePattern* fp = sm.getAgent()->getFirePattern();

	if (!ro || !fp || !target.isValid()) {
		sm.pop();
		return;
	}
	
	if (ro->isObjectVisible(target.get()) && sm.agent->space->isInPlayerRoom(sm.agent->getPos()))
	{
		if (fp->fireIfPossible()) {
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

BuildStressFromPlayerProjectiles::BuildStressFromPlayerProjectiles(float scale) :
	scale(scale)
{
}

void BuildStressFromPlayerProjectiles::onEnter(StateMachine& sm)
{
	sm.addDetectFunction(
		GType::playerBullet,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			sm.getAgent()->modifyAttribute(Attribute::stress, scale);
		}
	);
}

void BuildStressFromPlayerProjectiles::onExit(StateMachine& sm)
{
	sm.removeDetectFunction(GType::playerBullet);
}

FireOnStress::FireOnStress(float stressPerShot) :
	stressPerShot(stressPerShot)
{
}

void FireOnStress::update(StateMachine& sm)
{
	Agent* a = sm.getAgent();

	if (a->getAttribute(Attribute::stress) >= stressPerShot && a->getFirePattern()->fireIfPossible()) {
		a->modifyAttribute(Attribute::stress, -stressPerShot);
	}
}

}//end NS
