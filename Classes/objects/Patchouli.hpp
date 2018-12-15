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

class Patchouli : virtual public Agent, public NoAttributes
{
public:
    inline Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
    {}
    
    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 30.0;}
    virtual inline GType getType() const {return GType::npc;}
    
    inline string imageSpritePath() const {return "sprites/patchouli.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
};

#endif /* Patchouli_hpp */
