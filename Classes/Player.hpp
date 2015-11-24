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
    inline Player(const ValueMap& args) : GObject(args) {
        addUpdater(std::bind(&Player::checkControls, this));
    }
    
    inline float getSpeed() const{
        return 3;
    };

    inline float getMass() const {return 1;}
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GScene::Layer sceneLayer() const {return GScene::Layer::ground;}
    no_op(init);
    
    void checkControls();
};

#endif /* Player_hpp */
