//
//  Marisa.h
//  Koumachika
//
//  Created by Toni on 12/12/15.
//
//

#ifndef Marisa_h
#define Marisa_h

#include "Agent.hpp"

class Marisa : virtual public Agent, public NoAttributes
{
public:
    inline Marisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
	{}
    
    virtual inline float getRadarRadius() const {return 6.0f;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    
    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 33.0f;}
    virtual inline GType getType() const {return GType::npc;}
    
    inline string imageSpritePath() const {return "sprites/marisa.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	inline virtual string getScriptName() const { return "marisa"; }
};

#endif /* Marisa_h */
