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

#define cons(x) x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, const bullet_attributes& attributes);

class StarBullet : virtual public GObject, public Bullet, public CircleBody
{
public:
    static const vector<string> colors;

	StarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, const bullet_attributes& attributes, SpaceFloat speed, SpaceFloat radius, const string& color);

    const string color;

	virtual DamageInfo getDamageInfo() const;

	virtual inline SpaceFloat getKnockbackForce() const { return 150.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }
	virtual inline SpaceFloat getMaxSpeed() const { return 1.0; }

    virtual inline string getSprite() const {return "star-"+color;}
};

class IllusionDialDagger :
virtual public GObject,
public Bullet,
public RectangleBody
{
public:
    //IllusionDaggerBullet(const ValueMap& args);
    IllusionDialDagger(GSpace* space, ObjectIDType id, const bullet_attributes& attributes, const SpaceVect& pos, SpaceFloat anglar_velocity);

	virtual inline SpaceFloat getMaxSpeed() const { return 3.0; }
    
    virtual inline string getSprite() const {return "illusionDagger";}

	virtual void initializeGraphics();
    
    SpaceFloat targetViewAngle();
    void launch();

	virtual void update();
};

class ReimuBullet1 : public BulletImpl
{
public:
	static const string props;
	static const SpaceFloat omega;
	static const SpaceFloat amplitude;

	static SpaceVect parametric_move(
		SpaceFloat t,
		SpaceFloat angle,
		SpaceFloat phaseAngleStart,
		SpaceFloat speed
	);

	ReimuBullet1(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, const bullet_attributes& attributes, SpaceFloat start);
};

class YinYangOrb : public BulletImpl
{
public:
	static const string props;

	cons(YinYangOrb);
};

class RumiaDemarcation2Bullet : public BulletImpl
{
public:
	static const string props;

	RumiaDemarcation2Bullet(
		GSpace* space,
		ObjectIDType id,
		const SpaceVect& pos,
		SpaceFloat angle,
		const bullet_attributes& attributes,
		SpaceFloat angularVel
	);

	virtual void update();
};

class RumiaDarknessBullet : public BulletImpl
{
public:
	static const string props;

	RumiaDarknessBullet(
		GSpace* space,
		ObjectIDType id,
		const SpaceVect& pos,
		SpaceFloat angle,
		const bullet_attributes& attributes
	);
};


#undef cons

#endif /* Bullet_hpp */
