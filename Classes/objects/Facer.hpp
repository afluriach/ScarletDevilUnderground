//
//  Facer.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Facer_hpp
#define Facer_hpp

#include "Enemy.hpp"

class Facer : public Enemy, public BaseAttributes<Facer>
{
public:
	static const AttributeMap baseAttributes;

    inline Facer(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
    {}

    virtual void hit(int damage, shared_ptr<MagicEffect> effect);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/tewi.png";}
 
	virtual inline void initStateMachine(ai::StateMachine& sm) {
		sm.addThread(make_shared<ai::FacerMain>());
	}
};


#endif /* Facer_hpp */
