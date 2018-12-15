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

	inline Tewi(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		MapObjForwarding(Agent)
	{}

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	inline SpaceFloat getMass() const { return 40.0; }
	virtual inline GType getType() const { return GType::enemy; }

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi * 0.75; }

	inline string imageSpritePath() const { return "sprites/tewi.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};


#endif /* Tewi_hpp */
