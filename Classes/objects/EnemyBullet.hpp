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
	EnemyBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props);
	inline virtual ~EnemyBulletImpl() {}
};

class WaterBullet :
virtual public GObject,
public EnemyBullet,
public CircleBody,
public ImageSprite,
public LightObject,
public MaxSpeedImpl,
public DirectionalLaunch
{
public:

	WaterBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed);

	virtual inline shared_ptr<LightArea> getLightSource() const {
		return app::getLight("waterBullet");
	}

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string getSprite() const {return "waterBullet";}    
};

class FireBullet : virtual public GObject, public EnemyBullet, public CircleBody, public LoopAnimationSprite, public DirectionalLaunch, public MaxSpeedImpl
{
public:
	FireBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed);

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual string getSprite() const {return "fireBullet";}
    virtual float animationDuration() const {return 0.3f;}    
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

    virtual inline string getSprite() const {return "star-"+color;}
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
    
    virtual inline string getSprite() const {return "illusionDagger";}

	virtual void initializeGraphics();
    
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

class YinYangOrb : public EnemyBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	cons(YinYangOrb);
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

class RumiaDarknessBullet :
	public EnemyBullet,
	public ShieldBullet,
	public BulletImpl
{
public:
	static const bullet_properties props;

	RumiaDarknessBullet(
		GSpace* space,
		ObjectIDType id,
		const SpaceVect& pos,
		SpaceFloat angle,
		object_ref<Agent> agent
	);
};


#undef cons

#endif /* Bullet_hpp */
