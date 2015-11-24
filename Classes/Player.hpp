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
    inline Player(const ValueMap& args) : GObject(args) {}

    inline float getMass() const {return 1;}
    inline string imageSpritePath() const {return "sprites/flandre.png";}
    inline GScene::Layer sceneLayer() const {return GScene::Layer::ground;}
    no_op(init);
};

#endif /* Player_hpp */
