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
#include "GObjectMixins.hpp"
#include "macros.h"

class Collectible;

class Player : virtual public GObject,
public PatchConSprite,
public CircleBody,
public RadarObject,
public Spellcaster,
RegisterUpdate<Player>
{
public:
    static const int defaultMaxHealth;
    static const int defaultMaxPower;
    
    static const int batModeInitialCost;
    static const int batModeCostPerSecond;
    
    static const float fireDist;
    
    static const float interactCooldownTime;
    
    static const float spellCooldownTime;
    static const float hitProtectionTime;
    static const float hitFlickerInterval;
    
    static const float baseMaxSpeed;
    static const float batModeMaxSpeed;

    inline Player(const ValueMap& args) :
    GObject(args),
    PatchConSprite(args),
    RegisterUpdate<Player>(this)
    {}
    
    inline void setMaxSpeed(float s) {crntMaxSpeed = s;}
    virtual inline float getMaxSpeed() const{ return crntMaxSpeed;}
    virtual inline float getMaxAcceleration() const {return 6.0f;}
    
    //setting for player object sensing
	inline virtual float getRadarRadius() const { return 2.5f; }
	inline virtual GType getRadarType() const { return GType::objectSensor; }
    inline virtual float getDefaultFovAngle() const { return float_pi / 4.0f;}

    inline float getFireInterval() const {
        return 0.6f;
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
    
    inline bool isSpellProtectionMode() const {return spellProtectionMode;}
    inline void setSpellProtectionMode(bool mode) {spellProtectionMode = mode;}
    
    virtual void stop();
    
    void setMaxHealth(int val);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 20.0f;}
    virtual inline GType getType() const {return GType::player;}
    
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    void update();
    void updateFireTime();
    void updateHitTime();
    void checkBaseControls();
    void checkBatModeControls();
    void updateSpell();
    void fireIfPossible();
    void fire();
    
    void onCollectible(Collectible* coll);
protected:
    float hitProtectionCountdown = 0.0f;
    float spellCooldown = 0.0f;
    float interactCooldown = 0.0f;
    float lastFireTime = 0;
    
    float crntMaxSpeed = baseMaxSpeed;
    
    int maxHealth = defaultMaxHealth;
    int health = defaultMaxHealth;
    
    int maxPower = defaultMaxPower;
    int power = 100;
    
    bool spellProtectionMode = false;
};

#endif /* Player_hpp */
