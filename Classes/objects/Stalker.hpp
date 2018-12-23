//
//  Stalker.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Stalker_hpp
#define Stalker_hpp

#include "Agent.hpp"
#include "Collectibles.hpp"
#include "macros.h"

class Stalker : virtual public Agent, public TouchDamageEnemy, public BaseAttributes<Stalker>, public CollectibleDrop<Magic1>
{
public:
	static const AttributeMap baseAttributes;

    inline Stalker(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
    {}

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return float_pi / 2.0;}

    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 40.0;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/dark_cirno.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* Stalker_hpp */
