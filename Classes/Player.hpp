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

class Player : virtual public GObject, PatchConSprite, CircleBody, RegisterUpdate<Player>, ObjectSensor
{
public:
    static constexpr float fireDist = 1;
    static const int defaultMaxHealth = 5;
    static constexpr float hitProtectionTime = 2.4;

    inline Player(const ValueMap& args) : GObject(args), RegisterUpdate<Player>(this) {
    }
    
    virtual inline float getMaxSpeed() const{ return 3;}
    virtual inline float getMaxAcceleration() const {return 6;}

    inline float getFireInterval() const {
        return 0.6;
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
    
    void setMaxHealth(int val);

    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 20.0;}
    virtual inline GType getType() const {return GType::player;}
    
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    void updateFireTime();
    void updateHitTime();
    void checkControls();
    void fireIfPossible();
    void fire();
protected:
    float hitProtectionCountdown;
    float lastFireTime = 0;
    int maxHealth = defaultMaxHealth;
    int health = defaultMaxHealth;
};

#endif /* Player_hpp */
