//
//  Bomb.hpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#ifndef Bomb_hpp
#define Bomb_hpp

#include "Attributes.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Bomb :
	virtual public GObject,
	public CircleBody,
	public FrictionObject,
	public RegisterInit<Bomb>,
	public RegisterUpdate<Bomb>
{
public:
	Bomb(GSpace* space, ObjectIDType id, const SpaceVect& pos);
	inline virtual ~Bomb() {}

	virtual inline SpaceFloat uk() const { return 0.0625; }
	virtual inline bool getSensor() const { return false; }
	virtual inline SpaceFloat getMass() const { return 1.0; }
	virtual inline PhysicsLayers getLayers() const { return enum_bitwise_or( PhysicsLayers,ground,floor); }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
	virtual inline GType getType() const { return GType::bomb; }

	void init();
	void update();
	void detonate();
	float getScale(const GObject* target);
	void applyKnockback(GObject* target, SpaceFloat mag);

	//Interface
	virtual AttributeMap getAttributeEffect() const = 0;
	virtual inline shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }
	virtual SpaceFloat getFuseTime() const = 0;
	virtual SpaceFloat getBlastRadius() const = 0;
protected:
	SpaceFloat countdown;
};

class PlayerBomb : public Bomb, public ImageSprite
{
public:
	PlayerBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos);

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }
	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const {
		return { { Attribute::hp, -20.0f} };
	};

	virtual SpaceFloat getFuseTime() const { return 3.0; }
	virtual SpaceFloat getRadius() const { return 0.5; }
	virtual SpaceFloat getBlastRadius() const { return 6.0; }
};

#endif /* Bomb_hpp */
