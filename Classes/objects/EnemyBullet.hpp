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

#undef cons

#endif /* Bullet_hpp */
