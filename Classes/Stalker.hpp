//
//  Stalker.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Stalker_hpp
#define Stalker_hpp

#include "AI.hpp"
#include "AIMixins.hpp"
#include "Agent.hpp"

class Stalker : public Agent, virtual HitPointsEnemy, virtual TouchDamageEnemy, virtual PlayerBulletDamage
{
public:
    static const int maxHP;

    inline Stalker(const ValueMap& args) :
	GObject(args),
    Agent(args),
    HitPointsEnemy(maxHP)
    {}

    virtual inline float getRadarRadius() const {return 6.0f;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline float getDefaultFovAngle() const {return float_pi / 2.0f;}

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/dark_cirno.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
    virtual inline float getMaxSpeed() const {return 1.5f;}
    virtual inline float getMaxAcceleration() const {return 4.5f;}

	virtual inline shared_ptr<ai::State> getStartState() {
		return make_shared<ai::DetectAndSeekPlayer>();
	}        
};

#endif /* Stalker_hpp */
