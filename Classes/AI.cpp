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

void StateMachine::update()
{
    if(states.empty())
        return;
    State* crnt = states.back().get();
    
    crnt->update(*this);
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

void MoveToPoint::update(StateMachine& fsm)
{
    float dist2 = (fsm.agent->getPos() - target).lengthSq();
    
    if(dist2 < 0.25*0.25){
        fsm.pop();
		return;
    }
    
    seek(*fsm.agent, target, fsm.agent->getMaxSpeed(), fsm.agent->getMaxAcceleration());
}

void Wander::update(StateMachine& fsm)
{
    float dist = app->getRandomFloat(minDist, maxDist);
    vector<Direction> directions;
    SpaceVect target;
    
    target = dirToVector(Direction::up)*dist;
    if(GScene::getSpace()->obstacleFeeler(fsm.agent, target) == dist){
        directions.push_back(Direction::up);
    }
    target = dirToVector(Direction::right)*dist;
    if(GScene::getSpace()->obstacleFeeler(fsm.agent, target) == dist){
        directions.push_back(Direction::right);
    }
    target = dirToVector(Direction::down)*dist;
    if(GScene::getSpace()->obstacleFeeler(fsm.agent, target) == dist){
        directions.push_back(Direction::down);
    }
        target = dirToVector(Direction::left)*dist;
    if(GScene::getSpace()->obstacleFeeler(fsm.agent, target) == dist){
        directions.push_back(Direction::left);
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
