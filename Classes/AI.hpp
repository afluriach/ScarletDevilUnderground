//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject& obj, const SpaceVect& desired, float maxForce);
void seek(GObject& agent, GObject& target, float maxSpeed, float acceleration);
void seek(GObject& agent, const SpaceVect& target, float maxSpeed, float acceleration);
SpaceVect directionToTarget(GObject& agent, GObject& target);
SpaceVect directionToTarget(GObject& agent, const SpaceVect& target);

class StateMachine;

class State
{
public:
    virtual void onEnter(StateMachine& sm) = 0;
    virtual void update(StateMachine& sm) = 0;
    virtual void onExit(StateMachine& sm) = 0;
};

class StateMachine
{
public:
    GObject *const agent;

    inline StateMachine(GObject *const agent) :
    agent(agent)
    {
        frame = GScene::getSpace()->getFrame();
    }

    void update();
    void push(shared_ptr<State> newState);
    void pop();
    
    inline unsigned int getFrame(){
        return frame;
    }
    
protected:
    list<shared_ptr<State>> states;
    unsigned int frame;
};

class IdleWait : public State{
    public:
        inline IdleWait(unsigned int frames) :
        remaining(frames)
        {}
        inline virtual void onEnter(StateMachine& sm) {};
        inline virtual void onExit(StateMachine& sm) {};
    
        inline virtual void update(StateMachine& fsm){
            if(remaining == 0)
                fsm.pop();
            --remaining;
            
            ai::applyDesiredVelocity(*fsm.agent, SpaceVect(0,0), fsm.agent->getMaxAcceleration());
        }
    private:
        unsigned int remaining;
};

class MoveToPoint : public State{
public:
    inline MoveToPoint(SpaceVect target) :
    target(target)
    {}
    
    inline virtual void onEnter(StateMachine& sm) {};
    inline virtual void onExit(StateMachine& sm) {};
    
    virtual void update(StateMachine& fsm);
protected:
    SpaceVect target;
};

class Wander : public State {
public:
    inline Wander() : minWait(1.0), maxWait(3.0), minDist(2.0), maxDist(4.0)
    {}

    virtual void update(StateMachine& sm);
    
    inline virtual void onEnter(StateMachine& sm) {};
    inline virtual void onExit(StateMachine& sm) {};
protected:
    float minWait, maxWait;
    float minDist, maxDist;
};

class Operation : public State {
public:
    inline Operation(std::function<void(StateMachine&)> op) :
    op(op)
    {}

    inline virtual void update(StateMachine& sm){
        op(sm);
        sm.pop();
    }
    
    inline virtual void onEnter(StateMachine& sm) {};
    inline virtual void onExit(StateMachine& sm) {};
protected:
    std::function<void(StateMachine&)> op;
};

} //end NS

#endif /* AI_hpp */
