//
//  Facer.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Facer_hpp
#define Facer_hpp

class Facer :
virtual public GObject,
PatchConSprite,
CircleBody,
TouchDamageEnemy,
RegisterInit<Facer>,
RegisterUpdate<Facer>
{
public:
    inline Facer(const ValueMap& args) :
    GObject(args),
    PatchConSprite(args),
    RegisterInit<Facer>(this),
    RegisterUpdate<Facer>(this)
    {}

    virtual void onPlayerBulletHit(Bullet* bullet);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/tewi.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
    virtual inline float getMaxSpeed() const {return 1.0f;}
    virtual inline float getMaxAcceleration() const {return 4.5f;}
        
    GObject* target = nullptr;
    
    void init();
    void update();
};


#endif /* Facer_hpp */
