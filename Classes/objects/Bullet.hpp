//
//  Bullet.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

class Bullet : public GObject
{
public:
	static constexpr bool logRicochets = false;

	static object_params makeLaunchParams(
		SpaceVect pos,
		SpaceFloat angle,
		SpaceFloat speed,
		SpaceFloat angularVel,
		SpaceVect dimensions
	);

	static object_params makeParams(
		SpaceVect pos,
		SpaceFloat angle,
		SpaceVect vel,
		SpaceFloat angularVel,
		SpaceVect dimensions
	);

	Bullet(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		const bullet_attributes& attributes,
		local_shared_ptr<bullet_properties> props
	);
	~Bullet();

	virtual void onRemove();
	virtual void initializeGraphics();
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
	virtual SpaceFloat getMaxSpeed() const;

	DamageInfo getDamageInfo() const;

	virtual bool isInvisible() const;

	void onWallCollide(Wall* wall);
	void onEnvironmentCollide(GObject* obj);
	void onAgentCollide(Agent* agent, SpaceVect n);
	void onBulletCollide(Bullet* bullet);

	DamageInfo getScaledDamageInfo() const;
	bool applyRicochet(SpaceVect n);
	void setBodyVisible(bool b);
protected:
	bullet_attributes attributes;
	local_shared_ptr<bullet_properties> props;

	int ricochetCount = 0;
	int hitCount = 1;
};

#endif /* Bullet_hpp */
