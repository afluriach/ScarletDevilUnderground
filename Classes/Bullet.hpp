//
//  Bullet.hpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

#include "GObject.hpp"

using namespace std;

class Bullet : virtual public GObject, public CircleBody
{
public:
    //For now bullets can be treated as kinematic, meaning their mass is not relevant.
    virtual inline float getMass() const {return 0.1;}
    virtual inline bool getSensor() const {return true;}
};

class PlayerBaseBullet : virtual public Bullet, public ImageSprite
{
public:
    static constexpr float speed = 6;

    inline PlayerBaseBullet(float angle, const cp::Vect& pos) : angle(angle), GObject("playerBaseBullet", pos, cp::Vect(0.5,0.5)) {}

    virtual inline string imageSpritePath() const {return "sprites/flandre_bullet.png";}
    virtual inline GScene::Layer sceneLayer() const {return GScene::Layer::ground;}
    
    virtual inline GSpace::Type getType() const {return GSpace::Type::playerBullet;}
    virtual inline float getRadius() const {return 0.3;}
    
    void init();
protected:
    float angle;
};

#endif /* Bullet_hpp */
