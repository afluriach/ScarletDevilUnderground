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

class Stalker :
virtual public GObject,
PatchConSprite,
CircleBody,
RadarStateMachineObject,
virtual HitPointsEnemy,
TouchDamageEnemy,
PlayerBulletDamage
{
public:
    static const int maxHP;

    inline Stalker(const ValueMap& args) :
    GObject(args),
	RadarStateMachineObject(make_shared<ai::Seek>(),args),
    HitPointsEnemy(maxHP)
    {}

    virtual inline float getRadarRadius() const {return 6;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}

    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 40.0;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/dark_cirno.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
    virtual inline float getMaxSpeed() const {return 1.5f;}
    virtual inline float getMaxAcceleration() const {return 4.5;}
        
    Player* target = nullptr;
};

#endif /* Stalker_hpp */
