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
#include "App.h"
#include "FirePattern.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "macros.h"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"

namespace ai{

//It seems to better to base this on acceleration than force. In some cases mass may
//just be a default value, in which case acceleration makes more sense.
void applyDesiredVelocity(GObject* obj, SpaceVect desired, SpaceFloat acceleration)
{
	SpaceVect floorVel = obj->crntFloor.isValid() ? obj->crntFloor.get()->getVel() : SpaceVect::zero;
	SpaceFloat traction = obj->crntFloor.isValid() ? obj->crntFloor.get()->getFrictionCoeff() : 1.0;

	desired += floorVel;
	acceleration *= traction;

    //the desired velocity change
    SpaceVect vv = desired - obj->getVel();
    //the scalar amount of velocity change in one frame
	SpaceFloat dv = acceleration * App::secondsPerFrame;

    //Default case, apply maximum acceleration
    if(square(dv) < vv.lengthSq()){
		SpaceFloat f = obj->body->getMass() * acceleration;
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
	newState->onEnter(*sm);
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

void Thread::onDetect(StateMachine& sm, GObject* obj)
{
	if (!completed && !call_stack.empty())
	{
		call_stack.back()->onDetect(sm, obj);
	}
}

void Thread::onEndDetect(StateMachine& sm, GObject* obj)
{
	if (!completed && !call_stack.empty())
	{
		call_stack.back()->onEndDetect(sm, obj);
	}
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

StateMachine::StateMachine(GObject *const agent) :
agent(agent)
{
    frame = agent->space->getFrame();
}

void StateMachine::update()
{
    bitset<lockCount> locks;
    
	removeCompletedThreads();
    
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
    current_threads[thread->uuid] = thread;
    
    if(threads_by_priority.find(thread->priority) == threads_by_priority.end()){
        threads_by_priority[thread->priority] = list<unsigned int>();
    }
    threads_by_priority[thread->priority].push_back(thread->uuid);
}

unsigned int StateMachine::addThread(shared_ptr<Function> threadMain)
{
	auto t = make_shared<Thread>(
		threadMain,
		this,
		0,
		bitset<lockCount>()
	);

   addThread(t);
   return t->uuid;
}

void StateMachine::removeThread(unsigned int uuid)
{
    if(current_threads.find(uuid) != current_threads.end()){
        auto t = current_threads[uuid];
        threads_by_priority[t->priority].remove(uuid);
        current_threads.erase(uuid);
    }
}

void StateMachine::removeThread(const string& mainName)
{
    list<unsigned int> toRemove;

    for(auto it = current_threads.begin(); it != current_threads.end(); ++it)
    {
        if(it->second->getMainFuncName() == mainName){
            toRemove.push_back(it->second->uuid);
        }
    }
    
    for(unsigned int uuid: toRemove){
        removeThread(uuid);
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

void StateMachine::onDetect(GObject* obj)
{
    for(auto it = current_threads.begin(); it != current_threads.end(); ++it)
	{
		crntThread = it->second.get();
		it->second->onDetect(*this,obj);
	}
	crntThread = nullptr;
}
void StateMachine::onEndDetect(GObject* obj)
{
    for(auto it = current_threads.begin(); it != current_threads.end(); ++it)
	{
		crntThread = it->second.get();
		it->second->onEndDetect(*this,obj);
	}
	crntThread = nullptr;
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

Detect::Detect(const string& target_name, Generator nextState) :
target_name(target_name),
nextState(nextState)
{}

Detect::Detect(GSpace* space, const ValueMap& args)
{
    target_name = getStringOrDefault(args, "target_name", "player");
}

void Detect::onDetect(StateMachine& sm, GObject* obj)
{
    if(obj->getName() == target_name)
    {
        sm.getCrntThread()->push(nextState(obj));
    }
}

Seek::Seek(GSpace* space, const ValueMap& args) {
    target = getObjRefFromStringField(space, args, "target_name");
}

void Seek::onEndDetect(StateMachine& sm, GObject* other)
{
    if(target == other){
	sm.getCrntThread()->pop();
}
}

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
    
        if(crnt_distance > distance + margin){
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
	else
		ai::applyDesiredVelocity(sm.agent, SpaceVect::zero, sm.agent->getMaxAcceleration());
}

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

void Flee::onEndDetect(StateMachine& sm, GObject* other)
{
	sm.getCrntThread()->pop();
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

DetectAndSeekPlayer::DetectAndSeekPlayer() :
Detect(
    "player",
    [](GObject* target) -> shared_ptr<Function>{
        return make_shared<Seek>(target);
    }
)
{}

DetectAndSeekPlayer::DetectAndSeekPlayer(GSpace* space, const ValueMap& args) :
DetectAndSeekPlayer()
{}

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
void IdleWait::update(StateMachine& fsm)
{
	if (remaining == 0)
		fsm.getCrntThread()->pop();
	--remaining;

	ai::applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.agent->getMaxAcceleration());
}

LookAround::LookAround(boost::rational<int> secondsPerDirection, bool clockwise) :
secondsPerDirection(secondsPerDirection),
timeRemaining(secondsPerDirection),
clockwise(clockwise)
{

}

void LookAround::update(StateMachine& fsm)
{
	timerDecrement(timeRemaining);

	if (timeRemaining <= 0) {
		fsm.agent->rotate(float_pi / 2.0 * (clockwise ? 1.0 : -1.0));
		timeRemaining = secondsPerDirection;
	}
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

void MoveToPoint::update(StateMachine& fsm)
{
    SpaceFloat dist2 = (fsm.agent->getPos() - target).lengthSq();
    
    if(dist2 < 0.25*0.25){
        fsm.pop();
		return;
    }
    
    seek(fsm.agent, target, fsm.agent->getMaxSpeed(), fsm.agent->getMaxAcceleration());
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
	pattern.get()->update();
	pattern.get()->fireIfPossible();
}

Cast::Cast(string _spell_name, const ValueMap& _spell_args) :
spell_name(_spell_name),
spell_args(_spell_args)
{}

Cast::Cast(GSpace* space, const ValueMap& args)
{
    spell_name = getStringOrDefault(args, "spell_name", "");
    spell_args = getMap(args, "spell_args");
}


void Cast::onEnter(StateMachine& sm)
{
	sm.agent->cast(spell_name,spell_args);
}

void Cast::update(StateMachine& sm)
{    
    if(!sm.agent->isSpellActive())
        sm.pop();
}

void Cast::onExit(StateMachine& sm)
{
	sm.agent->stopSpell();
}

void FacerMain::onEnter(StateMachine& sm)
{
	target = sm.agent->space->getObject("player");
}

void FacerMain::update(StateMachine& sm)
{
    if(target.isValid()){
        if (isFacingTarget(sm.agent, target.get())){
            sm.agent->setVel(SpaceVect::ray(sm.agent->getMaxSpeed(), sm.agent->getAngle()));
        }
        else{
            sm.agent->setVel(SpaceVect::zero);
        }
    }
    else{
        sm.pop();
    }
}

void FollowerMain::onEnter(StateMachine& sm)
{
	target = sm.agent->space->getObject("player");
}


void FollowerMain::update(StateMachine& sm)
{
    if(target.isValid()){
        if (ai::isFacingTargetsBack(sm.agent, target.get())){
            sm.agent->setVel(SpaceVect::ray(sm.agent->getMaxSpeed(), sm.agent->getAngle()));
        }
        else{
            sm.agent->setVel(SpaceVect::zero);
        }
    }
    else{
        sm.pop();
    }
}

void SakuyaMain::onEnter(StateMachine& sm)
{

}

void SakuyaMain::update(StateMachine& sm)
{
    sm.push(make_shared<Cast>("IllusionDial", ValueMap()));
}

const SpaceFloat IllusionDash::scale = 2.5;
const SpaceFloat IllusionDash::opacity = 0.25;
const SpaceFloat IllusionDash::speed = 10.0;

IllusionDash::IllusionDash(SpaceVect _target) :
target(_target)
{}

IllusionDash::IllusionDash(GSpace* space, const ValueMap& args)
{
    auto t = args.at("target").asValueMap();
    
    target = SpaceVect(t.at("x").asFloat(), t.at("y").asFloat());
}

void IllusionDash::onEnter(StateMachine& sm)
{
    SpaceVect disp = displacementToTarget(sm.agent, target);
    
    sm.agent->setVel(disp.normalizeSafe()*speed);
    sm.agent->sprite->runAction(motionBlurStretch(disp.length()/speed, disp.toAngle(), opacity, scale));
}

void IllusionDash::update(StateMachine& sm)
{
    SpaceVect disp = displacementToTarget(sm.agent, target);
    sm.agent->setVel(disp.normalizeSafe()*speed);
    
    if(disp.lengthSq() < 0.125f){
        sm.agent->setVel(SpaceVect::zero);
        sm.pop();
    }
}

}//end NS
