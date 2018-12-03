//
//  Tewi.hpp
//  Koumachika
//
//  Created by Toni on 11/21/18.
//
//

#ifndef Tewi_hpp
#define Tewi_hpp

#include "Agent.hpp"
#include "macros.h"

class Tewi : virtual public Agent, public TouchDamageEnemy, public BaseAttributes<Tewi>
{
public:
	static const AttributeMap baseAttributes;

	inline Tewi(const ValueMap& args) :
		GObject(args),
		Agent(args)
	{}

	virtual inline float getRadius() const { return 0.35f; }
	inline float getMass() const { return 40.0f; }
	virtual inline GType getType() const { return GType::enemy; }

	virtual inline float getRadarRadius() const { return 6.0f; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline float getDefaultFovAngle() const { return float_pi * 0.75f; }

	inline string imageSpritePath() const { return "sprites/tewi.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};


#endif /* Tewi_hpp */
