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
#include "object_ref.hpp"
#include "Player.hpp"

class Marisa : virtual public Agent, public BaseAttributes<Marisa>
{
public:
	static const AttributeMap baseAttributes;

    inline Marisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
	{}
    
    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    
    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 33.0;}
    virtual inline GType getType() const {return GType::enemy;}
    
    inline string imageSpritePath() const {return "sprites/marisa.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& sm);
};

class MarisaMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(MarisaMain)
};

#endif /* Marisa_h */
