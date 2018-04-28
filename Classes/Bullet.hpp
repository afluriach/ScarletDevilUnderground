//
//  Bullet.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "types.h"

class Bullet : virtual public GObject
{
public:
	inline Bullet() {}
    //For now bullets can be treated as kinematic, meaning their mass is not relevant.
    virtual inline float getMass() const {return 0.1f;}
    virtual inline bool getSensor() const {return true;}
};

class CircleBullet : public Bullet, public CircleBody
{
public:
    CircleBullet(const ValueMap& args);
    inline CircleBullet() : radius(0.3f){}
    inline CircleBullet(float radius) : radius(radius){}
    
    virtual inline float getRadius() const {return radius;}
    const float radius;
};

class RectangleBullet : public Bullet, public RectangleBody
{
public:
//    SquareBullet(const ValueMap& args);
    inline RectangleBullet() : dimensions(SpaceVect::unit_square){}
    inline RectangleBullet(SpaceVect dimensions) :dimensions(dimensions){}
    
    virtual inline SpaceVect getDimensions() const {return dimensions;}
    
    const SpaceVect dimensions;
};

class PlayerBaseBullet : virtual public CircleBullet, public ImageSprite
{
public:

    PlayerBaseBullet(float angle, const SpaceVect& pos);
    
    virtual inline float getMaxSpeed() const {return 6.0f;}

    virtual inline string imageSpritePath() const {return "sprites/flandre_bullet.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::playerBullet;}
    
    static constexpr float spriteBaseRadius = 0.83f;
    inline virtual float zoom() const {return radius/spriteBaseRadius*2;}
};

class WaterBullet : virtual public CircleBullet, public ImageSprite
{
public:

    inline WaterBullet(const ValueMap& arg) : CircleBullet(arg), GObject(arg) {}

    virtual inline float getMaxSpeed() const {return 6.0f;}

    virtual inline string imageSpritePath() const {return "sprites/water_bullet.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::enemyBullet;}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return radius/spriteBaseRadius*2;}
};

class FireBullet : virtual public CircleBullet, public LoopAnimationSprite
{
public:
    inline FireBullet(const SpaceVect& pos) : GObject("stationaryFireBullet", pos) {
    }
    inline FireBullet(const ValueMap& arg) : CircleBullet(arg), GObject(arg) {}

    virtual string animationName() const {return "patchouli_fire";}
    virtual int animationSize() const {return 5.0f;}
    virtual float animationDuration() const {return 0.3f;}

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::enemyBullet;}
    
    static constexpr float spriteBaseRadius = 0.83f;
    inline virtual float zoom() const {return radius/spriteBaseRadius*2;}
};

class StarBullet : virtual public CircleBullet, public ImageSprite
{
public:
    static const vector<string> colors;

    inline StarBullet(const ValueMap& arg) :
    CircleBullet(arg),
    GObject(arg),
    color(arg.at("color").asString())
    {}
    inline StarBullet(const SpaceVect& pos, float radius, const string& color) :
    CircleBullet(radius),
    color(color),
    GObject("starBullet", pos)
    {}
    
    const string color;

    virtual inline string imageSpritePath() const {return "sprites/star-"+color+".png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::enemyBullet;}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return radius/spriteBaseRadius*2;}
};

class IllusionDialDagger : virtual public RectangleBullet, public ImageSprite
{
public:
    static const float speed;
    static const SpaceVect dimensions;

    //IllusionDaggerBullet(const ValueMap& args);
    IllusionDialDagger(const SpaceVect& pos, float anglar_velocity);
    
    virtual inline string imageSpritePath() const {return "sprites/knife green.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::enemyBullet;}
    
    inline virtual float zoom() const {return 1.0f;}
    
    float targetViewAngle();
    void launch();
};


#endif /* Bullet_hpp */
