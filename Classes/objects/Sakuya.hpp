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
public Agent,
virtual public HitPointsEnemy,
virtual public PlayerBulletDamage
{
public:
    static const int maxHP;

    inline Sakuya(const ValueMap& args) :
	GObject(args),
    Agent(args),
    HitPointsEnemy(maxHP)
    {}

    virtual inline float getRadarRadius() const {return 6.0f;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline float getDefaultFovAngle() const {return 0.0f;}

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/sakuya.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
    virtual float getMaxSpeed() const;
    virtual inline float getMaxAcceleration() const {return 4.5f;}

	virtual void initStateMachine(ai::StateMachine& sm);
};


#endif /* Sakuya_hpp */
