//
//  Fairy.hpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#ifndef Fairy_hpp
#define Fairy_hpp

#include "Agent.hpp"
#include "macros.h"

class Fairy1 :
virtual public Agent,
public BaseAttributes<Fairy1>
{
public:
	static const AttributeMap baseAttributes;

	inline Fairy1(const ValueMap& args) :
		GObject(args),
		Agent(args)
	{}

	virtual inline float getRadarRadius() const { return 6.0f; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline float getDefaultFovAngle() const { return float_pi / 4.0f; }

	virtual inline float getRadius() const { return 0.35f; }
	inline float getMass() const { return 40.0f; }
	virtual inline GType getType() const { return GType::enemy; }

	inline string imageSpritePath() const { return "sprites/melancholy.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Fairy2 :
	virtual public Agent,
	public BaseAttributes<Fairy2>
{
public:
	static const AttributeMap baseAttributes;

	inline Fairy2(const ValueMap& args) :
		GObject(args),
		Agent(args)
	{}

	virtual inline float getRadarRadius() const { return 6.0f; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline float getDefaultFovAngle() const { return float_pi / 4.0f; }

	virtual inline float getRadius() const { return 0.35f; }
	inline float getMass() const { return 40.0f; }
	virtual inline GType getType() const { return GType::enemy; }

	inline string imageSpritePath() const { return "sprites/melancholy.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);

	void sendCoverRequest(object_ref<Fairy2> other);
};

class IceFairy :
	virtual public Agent,
	public BaseAttributes<IceFairy>
{
public:
	static const AttributeMap baseAttributes;

	inline IceFairy(const ValueMap& args) :
		GObject(args),
		Agent(args)
	{}

	virtual inline float getRadarRadius() const { return 6.0f; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline float getDefaultFovAngle() const { return float_pi / 4.0f; }

	virtual inline float getRadius() const { return 0.35f; }
	inline float getMass() const { return 40.0f; }
	virtual inline GType getType() const { return GType::enemy; }

	inline string imageSpritePath() const { return "sprites/dark_cirno.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* Fairy_hpp */
