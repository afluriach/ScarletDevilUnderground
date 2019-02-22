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

class EnemyBullet : virtual public GObject, public Bullet
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

	WaterBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed);

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F(.375f,.75f,.75f,.5f), 0.0 }; }

    virtual inline SpaceFloat getMaxSpeed() const {return 6.0;}
	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string imageSpritePath() const {return "sprites/water_bullet.png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class FireBullet : virtual public GObject, public EnemyBullet, public CircleBody, public LoopAnimationSprite, public DirectionalLaunch, public MaxSpeedImpl
{
public:
	FireBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed);

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

	StarBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos, SpaceFloat speed, SpaceFloat radius, const string& color);

    const string color;

	virtual AttributeMap getAttributeEffect() const;

	virtual inline SpaceFloat getRadius() const { return 0.3; }

    virtual inline string imageSpritePath() const {return "sprites/star-"+color+".png";}
    
    static constexpr float spriteBaseRadius = 0.125f;
    inline virtual float zoom() const {return getRadius()/spriteBaseRadius*2;}
};

class Fairy1Bullet :
	virtual public GObject,
	public EnemyBullet,
	public CircleBody,
	public ImageSprite,
	public DirectionalLaunch
{
public:
	Fairy1Bullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 6.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/ice_fairy_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
	inline virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }
};

class GreenFairyBullet :
	virtual public GObject,
	public EnemyBullet,
	public CircleBody,
	public ImageSprite,
	public DirectionalLaunch
{
public:
	GreenFairyBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 6.0; }
	virtual inline SpaceFloat getRadius() const { return 0.15; }

	virtual inline string imageSpritePath() const { return "sprites/green_fairy_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
	inline virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }
};

class IceFairyBullet :
	virtual public GObject,
	public EnemyBullet,
	public CircleBody,
	public ImageSprite,
	public DirectionalLaunch
{
public:

	IceFairyBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 6.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/ice_fairy_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
	virtual shared_ptr<MagicEffect> getMagicEffect(gobject_ref target);
};

class LauncherBullet : virtual public GObject, public EnemyBullet, public CircleBody, public ImageSprite, public DirectionalLaunch
{
public:
	LauncherBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 10.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/launcher_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
	virtual inline shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }
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

class YinOrb :
	virtual public GObject,
	public EnemyBullet,
	public DirectionalLaunch,
	public CircleBody,
	public ImageSprite
{
public:
	YinOrb(GSpace* space, ObjectIDType id, Agent* agent, const SpaceVect& pos);

	virtual inline SpaceFloat getRadius() const { return 0.5; }
	virtual inline SpaceFloat getMaxSpeed() const { return 4.5; }
	virtual inline string imageSpritePath() const { return "sprites/yin-yang-orb.png"; }
	virtual inline float zoom() const { return 1.0f; }

	virtual AttributeMap getAttributeEffect() const;
};

class YangOrb : 
	virtual public GObject,
	public EnemyBullet,
	public DirectionalLaunch,
	public CircleBody,
	public ImageSprite
{
public:
	YangOrb(GSpace* space, ObjectIDType id, Agent* agent, const SpaceVect& pos);

	virtual inline SpaceFloat getRadius() const { return 0.5; }
	virtual inline SpaceFloat getMaxSpeed() const { return 4.5; }
	virtual inline string imageSpritePath() const { return "sprites/yin-yang-orb.png"; }
	virtual inline float zoom() const { return 1.0f; }
};


#endif /* Bullet_hpp */
