//
//  Facer.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Facer_hpp
#define Facer_hpp

#include "Agent.hpp"

class Facer : virtual public Agent, public TouchDamageEnemy, public BaseAttributes<Facer>
{
public:
	static const AttributeMap baseAttributes;

    inline Facer(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
    {}

    virtual void hit(int damage, shared_ptr<MagicEffect> effect);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/tewi.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
 
	virtual inline void initStateMachine(ai::StateMachine& sm) {
		sm.addThread(make_shared<ai::FacerMain>());
	}
};


#endif /* Facer_hpp */
