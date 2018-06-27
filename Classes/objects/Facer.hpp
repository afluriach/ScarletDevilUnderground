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

class Facer : public Agent, public TouchDamageEnemy
{
public:
    inline Facer(const ValueMap& args) :
	GObject(args),
    Agent(args)
    {}

    virtual void onPlayerBulletHit(Bullet* bullet);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/tewi.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
    virtual inline float getMaxSpeed() const {return 1.0f;}
    virtual inline float getMaxAcceleration() const {return 4.5f;}    

	virtual inline void initStateMachine(ai::StateMachine& sm) {
		sm.addThread(make_shared<ai::FacerMain>());
	}
};


#endif /* Facer_hpp */