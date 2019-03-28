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
	public RegisterInit<Bomb>
{
public:
	static const SpaceFloat explosionSpriteRadius;

	Bomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel);
	inline virtual ~Bomb() {}

	virtual inline SpaceFloat uk() const { return 0.0625; }
	virtual inline bool getSensor() const { return false; }
	virtual inline SpaceFloat getMass() const { return 1.0; }
	virtual inline PhysicsLayers getLayers() const { return enum_bitwise_or( PhysicsLayers,ground,floor); }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
	virtual inline GType getType() const { return GType::bomb; }

	void init();
	virtual void update();
	void detonate();

	//Interface
	virtual AttributeMap getAttributeEffect() const = 0;
	virtual SpaceFloat getFuseTime() const = 0;
	virtual SpaceFloat getBlastRadius() const = 0;
protected:
	SpaceFloat countdown;
};

class PlayerBomb : public Bomb, public ImageSprite
{
public:
	PlayerBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel);

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }
	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const {
		return { { Attribute::hp, -20.0f} };
	};

	virtual SpaceFloat getFuseTime() const { return 3.0; }
	virtual SpaceFloat getRadius() const { return 0.5; }
	virtual SpaceFloat getBlastRadius() const { return 4.0; }
};

class RedFairyBomb : public Bomb, public ImageSprite
{
public:
	RedFairyBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel);

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }
	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const {
		return { { Attribute::hp, -20.0f } };
	};

	virtual SpaceFloat getFuseTime() const { return 1.2; }
	virtual SpaceFloat getRadius() const { return 0.5; }
	virtual SpaceFloat getBlastRadius() const { return 3.0; }
};


#endif /* Bomb_hpp */
