//
//  Player.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Player_hpp
#define Player_hpp

class Player : virtual public GObject, PatchConSprite, CircleBody, RegisterUpdate<Player>
{
public:
    static constexpr float fireDist = 1;
    static const int maxHealth = 5;
    static constexpr float hitProtectionTime = 2.4;
    static constexpr float accel = 6;

    inline Player(const ValueMap& args) : GObject(args), RegisterUpdate<Player>(this) {
    }
    
    inline float getSpeed() const{
        return 3;
    };

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

    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
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
    int health = maxHealth;
};

#endif /* Player_hpp */
