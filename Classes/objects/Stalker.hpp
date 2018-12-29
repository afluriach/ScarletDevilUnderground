//
//  Stalker.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Stalker_hpp
#define Stalker_hpp

#include "Collectibles.hpp"
#include "Enemy.hpp"
#include "macros.h"

class Stalker : public Enemy, public BaseAttributes<Stalker>
{
public:
	static const AttributeMap baseAttributes;

    inline Stalker(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent),
	Enemy(collectible_id::magic1)
    {}

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return float_pi / 2.0;}

    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/dark_cirno.png";}

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* Stalker_hpp */
