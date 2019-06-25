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

#define cons(x) x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, const bullet_attributes& attributes);

class StarBullet : public Bullet
{
public:
    static const vector<string> colors;

	StarBullet(shared_ptr<object_params> params, const bullet_attributes& attributes,const string& color);

    const string color;

	virtual DamageInfo getDamageInfo() const;

	virtual inline SpaceFloat getKnockbackForce() const { return 150.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }
	virtual inline SpaceFloat getMaxSpeed() const { return 1.0; }

    virtual inline string getSprite() const {return "star-"+color;}
};

class IllusionDialDagger : public Bullet
{
public:
    //IllusionDaggerBullet(const ValueMap& args);
    IllusionDialDagger(shared_ptr<object_params> params, const bullet_attributes& attributes);

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

	static parametric_space_function getParametricFunction(SpaceVect origin, SpaceFloat angle, SpaceFloat tOffset);

	ReimuBullet1(shared_ptr<object_params> params, const bullet_attributes& attributes);
};

class YinYangOrb : public BulletImpl
{
public:
	static const string props;

	YinYangOrb(shared_ptr<object_params> params, const bullet_attributes& attributes);
};

class RumiaDemarcation2Bullet : public BulletImpl
{
public:
	static const string props;

	RumiaDemarcation2Bullet(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes
	);

	virtual void update();
};

class RumiaDarknessBullet : public BulletImpl
{
public:
	static const string props;

	RumiaDarknessBullet(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes
	);
};


#undef cons

#endif /* Bullet_hpp */
