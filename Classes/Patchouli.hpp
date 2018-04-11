//
//  Patchouli.hpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef Patchouli_hpp
#define Patchouli_hpp

#include "Agent.hpp"

class Patchouli : public Agent
{
public:
    inline Patchouli(const ValueMap& args) :
	GObject(args),
    Agent(args)
    {}
    
    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 30.0f;}
    virtual inline GType getType() const {return GType::enemy;}
    
    inline string imageSpritePath() const {return "sprites/patchouli.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
};

#endif /* Patchouli_hpp */
