//
//  Stalker.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Stalker_hpp
#define Stalker_hpp

class Stalker :
virtual public GObject,
virtual RadarObject,
PatchConSprite,
CircleBody,
RegisterUpdate<Stalker>,
virtual HitPointsEnemy,
TouchDamageEnemy,
PlayerBulletDamage
{
public:

    static const float maxSpeed;
    static const float acceleration;
    static const int maxHP;

    inline Stalker(const ValueMap& args) :
    GObject(args),
    RegisterUpdate<Stalker>(this),
    HitPointsEnemy(maxHP)
    {}

    virtual inline float getRadarRadius() const {return 6;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}

    virtual inline float getRadius() const {return 0.35;}
    inline float getMass() const {return 1;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/dark_cirno.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual void onDetect(GObject* other);
    virtual void onEndDetect(GObject* other);
    
    void update();
    
    Player* target = nullptr;
};

#endif /* Stalker_hpp */