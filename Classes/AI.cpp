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
}

void Seek::onDetect(StateMachine& sm, GObject* other)
{
	target = other;
}

void Seek::onEndDetect(StateMachine& sm, GObject* other)
{
	target = nullptr;
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
    
    for(Direction d=Direction::right; d< Direction::none; enum_increment(Direction, d) )
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

}//end NS
