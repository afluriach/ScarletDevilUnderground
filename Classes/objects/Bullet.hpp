//
//  Bullet.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

class Agent;
class Wall;

class Bullet : public GObject
{
public:
	static constexpr bool logRicochets = false;

	static local_shared_ptr<object_params> makeParams(
		SpaceVect pos,
		SpaceFloat angle,
		SpaceVect vel = SpaceVect::zero,
		SpaceFloat angularVel  = 0.0
	);

	Bullet(
		local_shared_ptr<object_params> params,
		const bullet_attributes& attributes,
		local_shared_ptr<bullet_properties> props
	);
	inline virtual ~Bullet() {}

	virtual void init();
	virtual void onRemove();
	virtual void initializeGraphics();
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
	virtual inline string getSprite() const { return props->sprite; }
	virtual inline boost::shared_ptr<LightArea> getLightSource() const { return app::getLight(props->lightSource); }
	virtual inline SpaceFloat getMaxSpeed() const { return props->speed; }

	inline DamageInfo getDamageInfo() const { return props->damage; }

	void onWallCollide(Wall* wall);
	void onEnvironmentCollide(GObject* obj);
	void onAgentCollide(Agent* agent, SpaceVect n);
	void onBulletCollide(Bullet* bullet);

	DamageInfo getScaledDamageInfo() const;
	SpaceVect calculateLaunchVelocity();
	bool applyRicochet(SpaceVect n);
	void setBodyVisible(bool b);
protected:
	bullet_attributes attributes;
	local_shared_ptr<bullet_properties> props;

	int ricochetCount = 0;
	int hitCount = 1;
};

#endif /* Bullet_hpp */
