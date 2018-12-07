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

class WaterBullet : virtual public GObject, public EnemyBullet, public CircleBody, public ImageSprite
{
public:

    inline WaterBullet(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
	{}

    virtual inline float getMaxSpeed() const {return 6.0f;}
	virtual inline float getRadius() const { return 0.3f; }

    virtual inline string imageSpritePath() const {return "sprites/water_bullet.png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class FireBullet : virtual public GObject, public EnemyBullet, public CircleBody, public LoopAnimationSprite, public DirectionalLaunch, public MaxSpeedImpl
{
public:
    inline FireBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos, float speed) :
		GObject(space, id, "fireBullet", pos, angle, true),
		MaxSpeedImpl(speed)
	{}

	virtual inline float getRadius() const { return 0.3f; }

    virtual string animationName() const {return "patchouli_fire";}
    virtual int animationSize() const {return 5.0f;}
    virtual float animationDuration() const {return 0.3f;}
    
    static constexpr float spriteBaseRadius = 0.83f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class StarBullet : virtual public GObject, public EnemyBullet, public CircleBody, public ImageSprite, public MaxSpeedImpl, public DirectionalLaunch
{
public:
    static const vector<string> colors;

    inline StarBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos, float speed, float radius, const string& color) :
    color(color),
	MaxSpeedImpl(speed),
    GObject(space,id,"starBullet", pos, angle,true)
    {}
    
    const string color;

	virtual inline float getRadius() const { return 0.3f; }

    virtual inline string imageSpritePath() const {return "sprites/star-"+color+".png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class IceFairyBullet : virtual public GObject, public EnemyBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	IceFairyBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos);

	virtual inline float getMaxSpeed() const { return 6.0f; }
	virtual inline float getRadius() const { return 0.3f; }

	virtual inline string imageSpritePath() const { return "sprites/ice_fairy_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target);

};


class IllusionDialDagger : virtual public GObject, public EnemyBullet, public RectangleBody, public ImageSprite
{
public:
    //IllusionDaggerBullet(const ValueMap& args);
    IllusionDialDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, float anglar_velocity);

	virtual inline float getMaxSpeed() const { return 3.0f; }
	virtual inline SpaceVect getDimensions() const { return SpaceVect(0.25f, 1.0f); }
    
    virtual inline string imageSpritePath() const {return "sprites/knife green.png";}
    
    inline virtual float zoom() const {return 1.0f;}
    
    float targetViewAngle();
    void launch();
};

class FlandreBigOrb1 : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	FlandreBigOrb1(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos);

	virtual inline float getMaxSpeed() const { return 4.5f; }
	virtual inline float getRadius() const { return 0.6f; }

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }
};

class FlandreFastOrb1 : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	FlandreFastOrb1(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos);

	virtual inline float getMaxSpeed() const { return 9.0f; }
	virtual inline float getRadius() const { return 0.15f; }

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }
};

class CirnoLargeIceBullet : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	CirnoLargeIceBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos);

	virtual inline float getMaxSpeed() const { return 9.0f; }
	virtual inline float getRadius() const { return 0.6; }

	virtual inline string imageSpritePath() const { return "sprites/cirno_large_ice_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target);
};

#endif /* Bullet_hpp */
