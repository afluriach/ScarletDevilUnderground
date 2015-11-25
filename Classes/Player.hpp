//
//  Player.hpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Player_hpp
#define Player_hpp

#include "GObject.hpp"

class Player : virtual public GObject, PatchConSprite, CircleBody
{
public:
    static constexpr float fireDist = 1;

    inline Player(const ValueMap& args) : GObject(args) {
        addUpdater(std::bind(&Player::playerUpdate, this));
    }
    
    inline float getSpeed() const{
        return 3;
    };

    inline float getFireInterval() const {
        return 0.6;
    }
    
    inline void playerUpdate(){
        updateFireTime();
        checkControls();
    }

    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
    virtual inline GSpace::Type getType() const {return GSpace::Type::player;}
    
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GScene::Layer sceneLayer() const {return GScene::Layer::ground;}
    no_op(init);
    
    void setDirection(Direction);
    void updateFireTime();
    void checkControls();
    void fireIfPossible();
    void fire();
protected:
    float lastFireTime = 0;
};

#endif /* Player_hpp */