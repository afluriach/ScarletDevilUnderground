//
//  Sakuya.hpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#ifndef Sakuya_hpp
#define Sakuya_hpp

#include "Agent.hpp"

class Sakuya :
virtual public Agent,
public BaseAttributes<Sakuya>
{
public:
	static const AttributeMap baseAttributes;

    inline Sakuya(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
    {}

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 40.0;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/sakuya.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& sm);
};


#endif /* Sakuya_hpp */
