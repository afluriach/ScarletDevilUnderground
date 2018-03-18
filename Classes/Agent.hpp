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
#include "AIMixins.hpp"

class Agent : virtual public GObject, PatchConSprite, CircleBody, RadarStateMachineObject
{
public:
    inline Agent(const ValueMap& args) :
    GObject(args),
    PatchConSprite(args),
    RadarStateMachineObject(make_shared<ai::WanderAndFleePlayer>(3.0f, this),args)
    {
        spriteName = args.at("sprite").asString();
        
    }
    
    virtual inline float getRadarRadius() const {return 3.0f;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline float getDefaultFovAngle() const {return 0.0f;}
    
    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 20.0;}
    virtual inline GType getType() const {return GType::enemy;}
    
    virtual inline float getMaxSpeed() const {return 1.5;};
    virtual inline float getMaxAcceleration() const {return 6;}
    
    inline string imageSpritePath() const {return "sprites/"+spriteName+".png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
protected:
    string spriteName;
};

#endif /* Agent_hpp */
