//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

#include "AI.hpp"

class Agent : virtual public GObject, PatchConSprite, CircleBody, RegisterUpdate<Agent>
{
public:
    inline Agent(const ValueMap& args) :
    GObject(args),
    fsm(this),
    RegisterUpdate<Agent>(this)
    {
        spriteName = args.at("sprite").asString();
        
        fsm.push(make_shared<ai::Wander>());
    }
    
    inline void update(){
        fsm.update();
    }
    
    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
    virtual inline GType getType() const {return GType::enemy;}
    
    virtual inline float getMaxSpeed() const {return 1.5;};
    virtual inline float getMaxAcceleration() const {return 6;}
    
    inline string imageSpritePath() const {return "sprites/"+spriteName+".png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
protected:
    string spriteName;
    ai::StateMachine fsm;
};

#endif /* Agent_hpp */
