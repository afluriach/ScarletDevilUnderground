//
//  EnemyBullet.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef EnemyBullet_hpp
#define EnemyBullet_hpp

#include "Bullet.hpp"
#include "GObjectMixins.hpp"

#define cons(x) x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent);

class EnemyBullet : virtual public Bullet
{
public:
	EnemyBullet(Agent* agent);
	inline virtual ~EnemyBullet() {}

	void invalidateGraze();
	virtual inline GType getType() const { return GType::enemyBullet; }
	bool grazeValid = true;
};

class WaterBullet :
virtual public GObject,
public EnemyBullet,
public CircleBody,
public ImageSprite,
public RadialLightObject,
public MaxSpeedImpl,
public DirectionalLaunch
{
public:

	WaterBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, SpaceFloat speed);

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F(.375f,.75f,.75f,.5f), 0.0 }; }

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string imageSpritePath() const {return "sprites/water_bullet.png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class FireBullet : virtual public GObject, public EnemyBullet, public CircleBody, public LoopAnimationSprite, public DirectionalLaunch, public MaxSpeedImpl
{
public:
	FireBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, SpaceFloat speed);

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual string animationName() const {return "patchouli_fire";}
    virtual int animationSize() const {return 5;}
    virtual float animationDuration() const {return 0.3f;}
    
    static constexpr float spriteBaseRadius = 0.83f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class StarBullet : virtual public GObject, public EnemyBullet, public CircleBody, public ImageSprite, public MaxSpeedImpl, public DirectionalLaunch
{
public:
    static const vector<string> colors;

	StarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, SpaceFloat speed, SpaceFloat radius, const string& color);

    const string color;

	virtual AttributeMap getAttributeEffect() const;

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string imageSpritePath() const {return "sprites/star-"+color+".png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class Fairy1Bullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(Fairy1Bullet);
};

class GreenFairyBullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(GreenFairyBullet);
};

class IceFairyBullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(IceFairyBullet);
};

class LauncherBullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(LauncherBullet);
};

class RumiaBullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(RumiaBullet);
};

class RumiaPinwheelBullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(RumiaPinwheelBullet);
};

class RumiaDemarcationBullet : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(RumiaDemarcationBullet);
};

class IllusionDialDagger :
virtual public GObject,
public EnemyBullet,
public RectangleBody,
public ImageSprite,
public RegisterUpdate<IllusionDialDagger>
{
public:
    //IllusionDaggerBullet(const ValueMap& args);
    IllusionDialDagger(GSpace* space, ObjectIDType id, Agent* agent, const SpaceVect& pos, SpaceFloat anglar_velocity);

	virtual inline SpaceFloat getMaxSpeed() const { return 3.0; }
    
    virtual inline string imageSpritePath() const {return "sprites/knife green.png";}

	virtual void initializeGraphics();
    
    inline virtual float zoom() const {return 1.0f;}
    
    SpaceFloat targetViewAngle();
    void launch();

	void update();
};

class YinYangOrb : public EnemyBullet, public BulletImpl, public SpriteLightObject
{
public:
	static const bullet_properties props;

	cons(YinYangOrb);

	virtual SpriteLightArea getLightSource() const;
};

#undef cons

#endif /* Bullet_hpp */
