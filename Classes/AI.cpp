//
//  AI.cpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#include "Prefix.h"
#include "AI.hpp"

namespace ai{

//It seems to better to base this on acceleration than force. In some cases mass may
//just be a default value, in which case acceleration makes more sense.
void applyDesiredVelocity(GObject& obj, const SpaceVect& desired, float acceleration)
{
    //the desired velocity change
    SpaceVect vv = desired - obj.getVel();
    //the scalar amount of velocity change in one frame
    float dv = acceleration * App::secondsPerFrame;

    //Default case, apply maximum acceleration
    if(square(dv) < vv.lengthSq()){
        float f = obj.body->getMass() * acceleration;
        SpaceVect ff = f * vv.normalizeSafe();
        obj.applyForceForSingleFrame(ff);
    }
    else{
        obj.setVel(desired);
    }
}

bool isFacingTarget(GObject* agent, GObject* target)
{
    SpaceVect targetDirection = (target->getPos() - agent->getPos()).normalize();
    SpaceVect agentFacingVector = agent->getFacingVector();
    bool facing = SpaceVect::dot(agentFacingVector, target->getFacingVector()) < 0;
    bool targetInFrontOfAgent = SpaceVect::dot(agentFacingVector, targetDirection) > 0;

    return facing && targetInFrontOfAgent;
}

bool isFacingTargetsBack(GObject* agent, GObject* target)
{
    SpaceVect targetDirection = (target->getPos() - agent->getPos()).normalize();
    SpaceVect agentFacingVector = agent->getFacingVector();

    bool facingBack = SpaceVect::dot(agentFacingVector, target->getFacingVector()) > 0;
    bool targetInFrontOfAgent = SpaceVect::dot(agentFacingVector, targetDirection) > 0;
    
    return facingBack && targetInFrontOfAgent;
}

SpaceVect directionToTarget(GObject& agent, GObject& target)
{
    return directionToTarget(agent, target.getPos());
}

SpaceVect directionToTarget(GObject& agent, const SpaceVect& target)
{
    return (target - agent.getPos()).normalize();
}

void seek(GObject& agent, GObject& target, float maxSpeed, float acceleration)
{
	seek(agent, target.getPos(), maxSpeed, acceleration);
}

void seek(GObject& agent, const SpaceVect& target, float maxSpeed, float acceleration)
{
	SpaceVect displacement = target - agent.getPos();

	if (displacement.lengthSq() < 1e-4)
		return;

    SpaceVect direction = directionToTarget(agent,target);
    
    applyDesiredVelocity(agent, direction*maxSpeed, acceleration);
}

void flee(GObject& agent, GObject& target, float maxSpeed, float acceleration)
{
	flee(agent, target.getPos(), maxSpeed, acceleration);
}

void flee(GObject& agent, const SpaceVect& target, float maxSpeed, float acceleration)
{
	SpaceVect displacement = target - agent.getPos();

	if (displacement.lengthSq() < 1e-4)
		return;

    displacement = displacement.normalize();
    displacement = displacement.rotate(float_pi);
    
    applyDesiredVelocity(agent, displacement*maxSpeed, acceleration);
}

shared_ptr<State> State::constructState(const string& type, const ValueMap& args)
{
    auto it = State::adapters.find(type);

    if (it != State::adapters.end()) {
        State::AdapterType adapter = it->second;
        return adapter(args);
    }
    else return nullptr;
}

void StateMachine::update()
{
    if(states.empty())
        return;
    State* crnt = states.back().get();
    
    crnt->update(*this);
}

void StateMachine::onDetect(GObject* obj)
{
	if (states.empty())
		return;
	State* crnt = states.back().get();

	crnt->onDetect(*this, obj);
}
void StateMachine::onEndDetect(GObject* obj)
{
	if (states.empty())
		return;
	State* crnt = states.back().get();

	crnt->onEndDetect(*this, obj);
}

void StateMachine::setState(shared_ptr<State> newState)
{
	clearState();
	push(newState);
}

void StateMachine::clearState()
{
	while (!states.empty())
	{
		pop();
	}
}

void StateMachine::clearSubstates()
{
	while (states.size() > 1)
	{
		pop();
	}
}

void StateMachine::push(shared_ptr<State> newState)
{
    states.push_back(newState);
    newState->onEnter(*this);
}

void StateMachine::pop()
{
    if(states.empty())
        return;
    State* crnt = states.back().get();
    
    crnt->onExit(*this);
    states.pop_back();
    
    if(!states.empty())
        states.back()->onReturn(*this);
}

Seek::Seek(const ValueMap& args) {
    if(args.find("target_name") == args.end()){
        log("Seek::Seek: target_name missing.");
    }
    target = GScene::getSpace()->getObject(args.at("target_name").asString());
    
    if(!target){
        log("Seek::Seek: target object %s not found.", args.at("target_name").asString().c_str() );
    }
}

void Seek::onEndDetect(StateMachine& sm, GObject* other)
{
	sm.pop();
}

void Seek::update(StateMachine& sm)
{
	if (target != nullptr) {
		ai::seek(
			*sm.agent,
			*target,
			sm.agent->getMaxSpeed(),
			sm.agent->getMaxAcceleration()
		);
		sm.agent->setDirection(toDirection(ai::directionToTarget(*sm.agent, *target)));
	}
	else
		ai::applyDesiredVelocity(*sm.agent, SpaceVect(0, 0), sm.agent->getMaxAcceleration());
}

Flee::Flee(const ValueMap& args) {
    if(args.find("target_name") == args.end()){
        log("Seek::Seek: target_name missing.");
    }
    target = GScene::getSpace()->getObject(args.at("target_name").asString());
    
    if(!target){
        log("Flee::Flee: target object %s not found.", args.at("target_name").asString().c_str() );
    }
    
    if(args.find("flee_distance") == args.end()){
        log("Flee::Flee: flee_distance missing.");
    }
    else{
        distance = args.at("flee_distance").asFloat();
    }
}

void Flee::onEndDetect(StateMachine& sm, GObject* other)
{
	sm.pop();
}

void Flee::update(StateMachine& sm)
{
	if (target != nullptr) {
		ai::flee(
			*sm.agent,
			*target,
			sm.agent->getMaxSpeed(),
			sm.agent->getMaxAcceleration()
		);
		sm.agent->setDirection(toDirection(ai::directionToTarget(*sm.agent, *target)));
	}
	else
		ai::applyDesiredVelocity(*sm.agent, SpaceVect(0, 0), sm.agent->getMaxAcceleration());
}


IdleWait::IdleWait(const ValueMap& args)
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
    
    float waitSeconds = getFloat(args, "waitTime");
    remaining = App::framesPerSecond * waitSeconds;
}
void IdleWait::update(StateMachine& fsm)
{
	if (remaining == 0)
		fsm.pop();
	--remaining;

	ai::applyDesiredVelocity(*fsm.agent, SpaceVect(0, 0), fsm.agent->getMaxAcceleration());
}



MoveToPoint::MoveToPoint(const ValueMap& args)
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
    float dist2 = (fsm.agent->getPos() - target).lengthSq();
    
    if(dist2 < 0.25*0.25){
        fsm.pop();
		return;
    }
    
    seek(*fsm.agent, target, fsm.agent->getMaxSpeed(), fsm.agent->getMaxAcceleration());
}

FollowPath::FollowPath(const ValueMap& args)
{
	auto name_it = args.find("pathName");
	auto loop_it = args.find("loop");

	if (name_it == args.end()) {
		log("FollowPath: pathName not provided!");
	}

	Path* p = GScene::getSpace()->getPath(name_it->second.asString());

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
		fsm.agent->setDirection(toDirection(ai::directionToTarget(*fsm.agent, path[currentTarget])));
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

Wander::Wander(const ValueMap& args) :
    init_float_field(minWait,1.0f),
    init_float_field(maxWait,1.0f),
    init_float_field(minDist,1.0f),
    init_float_field(maxDist,1.0f)
{}


void Wander::update(StateMachine& fsm)
{
    float dist = app->getRandomFloat(minDist, maxDist);
    vector<Direction> directions;
    SpaceVect target;
    
    enum_foreach(Direction,d,right,end)
    {
        target = dirToVector(d)*dist;
        if(!GScene::getSpace()->obstacleFeeler(fsm.agent, target)){
            directions.push_back(d);
        }
    }

    if(directions.empty())
        return;

    int randomIdx = app->getRandomInt(0, directions.size()-1);
    
    fsm.push(make_shared<MoveToPoint>(fsm.agent->getPos()+dirToVector(directions[randomIdx])*dist));
    
    PatchConSprite* spriteObject = dynamic_cast<PatchConSprite*>(fsm.agent);
    if(spriteObject){
        fsm.push(make_shared<Operation>(
            [=](StateMachine& sm) -> void {
                spriteObject->setDirection(directions[randomIdx]);
        }));
    }
    
    int waitFrames = app->getRandomInt(minWait*App::framesPerSecond, maxWait*App::framesPerSecond);
    fsm.push(make_shared<IdleWait>(waitFrames));
}

void FacerState::onEnter(StateMachine& sm)
{
	target = GScene::getSpace()->getObject("player");
}

void FacerState::update(StateMachine& sm)
{
	if (isFacingTarget(sm.agent, target))
	{
		sm.agent->setVel(SpaceVect::ray(sm.agent->getMaxSpeed(), sm.agent->getAngle()));
	}
	else
	{
		sm.agent->setVel(SpaceVect(0, 0));
	}
}

void FollowerState::onEnter(StateMachine& sm)
{
	target = GScene::getSpace()->getObject("player");
}


void FollowerState::update(StateMachine& sm)
{
	if (ai::isFacingTargetsBack(sm.agent, target))
	{
		sm.agent->setVel(SpaceVect::ray(sm.agent->getMaxSpeed(), sm.agent->getAngle()));
	}
	else
	{
		sm.agent->setVel(SpaceVect(0, 0));
	}
}


#define compound_method(name, args1, args2) \
void CompoundState::name args1 { \
    stateA->name args2; \
    stateB->name args2; \
}


compound_method(onEnter, (StateMachine& sm), (sm))
compound_method(onExit, (StateMachine& sm), (sm))
compound_method(update, (StateMachine& sm), (sm))
compound_method(onDetect,(StateMachine& sm, GObject* obj), (sm,obj))
compound_method(onEndDetect,(StateMachine& sm, GObject* obj), (sm,obj))

}//end NS
