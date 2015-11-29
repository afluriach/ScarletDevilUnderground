//
//  Bullet.hpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

class Bullet : virtual public GObject, public CircleBody
{
public:
    //For now bullets can be treated as kinematic, meaning their mass is not relevant.
    virtual inline float getMass() const {return 0.1;}
    virtual inline bool getSensor() const {return true;}
};

class PlayerBaseBullet : virtual public Bullet, public ImageSprite, RegisterInit<PlayerBaseBullet>
{
public:
    static constexpr float speed = 6;

    inline PlayerBaseBullet(float angle, const SpaceVect& pos) : angle(angle), GObject("playerBaseBullet", pos), RegisterInit<PlayerBaseBullet>(this) {}

    virtual inline string imageSpritePath() const {return "sprites/flandre_bullet.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::playerBullet;}
    virtual inline float getRadius() const {return 0.3;}
    
    void init();
protected:
    float angle;
};

class StationaryFireBullet : virtual public Bullet, public LoopAnimationSprite
{
public:
    inline StationaryFireBullet(const SpaceVect& pos) : GObject("stationaryFireBullet", pos) {
    }

    virtual string animationName() const {return "patchouli_fire";}
    virtual int animationSize() const {return 5;}
    virtual float animationDuration() const {return 0.3;}

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::enemyBullet;}
    virtual inline float getRadius() const {return 0.3;}
};

class PatchouliFireBullet : virtual public Bullet, public LoopAnimationSprite
{
public:
    static constexpr float speed = 6;

    inline PatchouliFireBullet(float angle, const SpaceVect& pos) : angle(angle), GObject("patchouliFireBullet", pos) {
        multiInit += bind(&PatchouliFireBullet::init, this);
    }

    virtual string animationName() const {return "patchouli_fire";}
    virtual int animationSize() const {return 5;}
    virtual float animationDuration() const {return 0.3;}

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::enemyBullet;}
    virtual inline float getRadius() const {return 0.3;}
    
    void init();
protected:
    float angle;
};

#endif /* Bullet_hpp */
