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

#define cons(x) x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent);

class EnemyBullet : virtual public Bullet
{
public:
	inline EnemyBullet() {}
	inline virtual ~EnemyBullet() {}

	virtual inline GType getType() const { return GType::enemyBullet; }
};

class EnemyBulletImpl : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties fairy1Bullet;
	static const bullet_properties greenFairyBullet;
	static const bullet_properties blueFairyBullet;
	static const bullet_properties iceFairyBullet;
	static const bullet_properties launcherBullet;
	static const bullet_properties rumiaBullet;
	static const bullet_properties rumiaDemarcationBullet;
	static const bullet_properties rumiaPinwheelBullet;

	EnemyBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, const bullet_properties* props);
	inline virtual ~EnemyBulletImpl() {}
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

	WaterBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed);

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F(.375f,.75f,.75f,.5f), 0.0 }; }

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string imageSpritePath() const {return "sprites/water_bullet.png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class FireBullet : virtual public GObject, public EnemyBullet, public CircleBody, public LoopAnimationSprite, public DirectionalLaunch, public MaxSpeedImpl
{
public:
	FireBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed);

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

	StarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed, SpaceFloat radius, const string& color);

    const string color;

	virtual DamageInfo getDamageInfo() const;

	virtual inline SpaceFloat getKnockbackForce() const { return 150.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string imageSpritePath() const {return "sprites/star-"+color+".png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class IllusionDialDagger :
virtual public GObject,
public EnemyBullet,
public RectangleBody,
public ImageSprite
{
public:
    //IllusionDaggerBullet(const ValueMap& args);
    IllusionDialDagger(GSpace* space, ObjectIDType id, object_ref<Agent> agent, const SpaceVect& pos, SpaceFloat anglar_velocity);

	virtual inline SpaceFloat getMaxSpeed() const { return 3.0; }
    
    virtual inline string imageSpritePath() const {return "sprites/knife green.png";}

	virtual void initializeGraphics();
    
    inline virtual float zoom() const {return 1.0f;}
    
    SpaceFloat targetViewAngle();
    void launch();

	virtual void update();
};

class ReimuBullet1 : public EnemyBullet, public BulletImpl, public ParametricMotion
{
public:
	static const bullet_properties props;
	static const SpaceFloat omega;
	static const SpaceFloat amplitude;

	static SpaceVect parametric_move(SpaceFloat t, SpaceFloat angle, SpaceFloat phaseAngleStart);

	ReimuBullet1(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat start);

	virtual void update();
};

class YinYangOrb : public EnemyBullet, public BulletImpl, public SpriteLightObject
{
public:
	static const bullet_properties props;

	cons(YinYangOrb);

	virtual SpriteLightArea getLightSource() const;
};

class RumiaDemarcation2Bullet :
	public EnemyBullet,
	public ShieldBullet,
	public BulletImpl
{
public:
	static const bullet_properties props;

	RumiaDemarcation2Bullet(
		GSpace* space,
		ObjectIDType id,
		const SpaceVect& pos,
		SpaceFloat angle,
		object_ref<Agent> agent,
		SpaceFloat angularVel
	);

	virtual void update();
};

#undef cons

#endif /* Bullet_hpp */
