//
//  Player.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Player_hpp
#define Player_hpp

#include "AIMixins.hpp"
#include "GObject.hpp"
#include "macros.h"

class Collectible;

class Player : virtual public GObject, PatchConSprite, CircleBody, RegisterUpdate<Player>, RadarObject
{
public:
    static constexpr float fireDist = 1.0f;
    static const int defaultMaxHealth = 5;
    static const int defaultMaxPower = 500;
    static constexpr float hitProtectionTime = 2.4f;
    static constexpr float hitFlickerInterval = 0.3f;

    inline Player(const ValueMap& args) :
    GObject(args),
    PatchConSprite(args),
    RegisterUpdate<Player>(this)
    {}
    
    virtual inline float getMaxSpeed() const{ return 3.0f;}
    virtual inline float getMaxAcceleration() const {return 6.0f;}
    
    //setting for player object sensing
	inline virtual float getRadarRadius() const { return 2.5f; }
	inline virtual GType getRadarType() const { return GType::objectSensor; }
    inline virtual float getDefaultFovAngle() const { return float_pi / 4.0f;}

    inline float getFireInterval() const {
        return 0.6f;
    }
    
    inline void update(){
        updateFireTime();
        updateHitTime();
        checkControls();
    }
    
    void hit();

    inline int getHealth(){
        return health;
    }
    
    inline void setHealth(int val){
        health = val;
    }
    
    inline int getMaxHealth(){
        return maxHealth;
    }
    
    inline int getPower(){
        return power;
    }
    
    inline bool consumePower(int val){
        if(power >= val){
            power -= val;
            return true;
        }
        return false;
    }
    
    void setMaxHealth(int val);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 20.0f;}
    virtual inline GType getType() const {return GType::player;}
    
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    void updateFireTime();
    void updateHitTime();
    void checkControls();
    void fireIfPossible();
    void fire();
    
    void onCollectible(Collectible* coll);
protected:
    float hitProtectionCountdown;
    float lastFireTime = 0;
    
    int maxHealth = defaultMaxHealth;
    int health = defaultMaxHealth;
    
    int maxPower = defaultMaxPower;
    int power = 100;
};

#endif /* Player_hpp */
