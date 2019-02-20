//
//  PlayerBullet.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef PlayerBullet_hpp
#define PlayerBullet_hpp

#include "Bullet.hpp"

class PlayerBullet : virtual public GObject, public Bullet
{
public:
	PlayerBullet(Agent* agent);
	inline virtual ~PlayerBullet() {}

	virtual void onAgentCollide(Agent* agent);

	virtual inline GType getType() const { return GType::playerBullet; }
};

class PlayerShield : virtual public GObject, public Bullet
{
public:
	PlayerShield(Agent* agent);
	inline virtual ~PlayerShield() {}

	virtual inline GType getType() const { return GType::playerBullet; }

	//Shield bullet is no-collide with normal obstacles, and is not consumed
	//upon contactwith an enemy.
	virtual void onWallCollide(Wall* wall);
	virtual void onEnvironmentCollide(GObject* obj);
	virtual void onAgentCollide(Agent* agent);
	virtual void onBulletCollide(Bullet* bullet);

	virtual SpaceFloat getKnockbackForce() const = 0;
};

class FlandreBigOrb1 : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	FlandreBigOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 4.5; }
	virtual inline SpaceFloat getRadius() const { return 0.6; }

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
};

class FlandreFastOrb1 :
	virtual public GObject,
	public PlayerBullet,
	public CircleBody,
	public ImageSprite,
	public DirectionalLaunch,
	public RadialLightObject
{
public:
	FlandreFastOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F::RED*0.5f, 0.0 }; }

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.15; }

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
};

class FlandrePolarMotionOrb :
	virtual public GObject,
	public PlayerBullet,
	public CircleBody,
	public ImageSprite,
	public DirectionalLaunch,
	public RadialLightObject,
	public RegisterUpdate<FlandrePolarMotionOrb>
{
public:
	FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	void update();

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F::RED*0.5f, 0.0 }; }

	virtual inline SpaceFloat getMass() const { return 1.0; }
	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.15; }

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
};

class FlandreCounterClockBullet :
	virtual public GObject,
	public PlayerShield,
	public RectangleBody,
	public ImageSprite
{
public:
	FlandreCounterClockBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline string imageSpritePath() const { return "sprites/counter_clock_bullet.png"; }
	virtual inline float zoom() const { return 0.25f; }

	virtual AttributeMap getAttributeEffect() const;
	virtual inline SpaceFloat getKnockbackForce() const { return 0.0; }
};

class RumiaFastOrb1 : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	RumiaFastOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.15; }

	virtual inline string imageSpritePath() const { return "sprites/rumia_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
};

class CirnoSmallIceBullet : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:
	CirnoSmallIceBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/cirno_large_ice_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual void onAgentCollide(Agent* agent);

	virtual AttributeMap getAttributeEffect() const;
	virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target);
protected:
	int hitsRemaining = 3;
};


class CirnoLargeIceBullet : virtual public GObject, public PlayerBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:

	CirnoLargeIceBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.6; }

	virtual inline string imageSpritePath() const { return "sprites/cirno_large_ice_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
	virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target);
};

class CirnoIceShieldBullet :
	virtual public GObject,
	public PlayerShield,
	public CircleBody,
	public ImageSprite
{
public:
	CirnoIceShieldBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/cirno_large_ice_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;

	inline virtual SpaceFloat getKnockbackForce() const { return 99.0; }
};

#endif /* PlayerBullet_hpp */
