//
//  Tewi.hpp
//  Koumachika
//
//  Created by Toni on 11/21/18.
//
//

#ifndef Tewi_hpp
#define Tewi_hpp

#include "Enemy.hpp"

class Tewi : public Enemy, public BaseAttributes<Tewi>
{
public:
	static const AttributeMap baseAttributes;

	inline Tewi(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		MapObjForwarding(Agent),
		Enemy(collectible_id::nil)
	{}

	inline SpaceFloat getMass() const { return 40.0; }

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string imageSpritePath() const { return "sprites/tewi.png"; }

	virtual void initStateMachine();
};


#endif /* Tewi_hpp */
