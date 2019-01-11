//
//  SakuyaNPC.hpp
//  Koumachika
//
//  Created by Toni on 12/27/18.
//
//

#ifndef SakuyaNPC_hpp
#define SakuyaNPC_hpp

#include "Agent.hpp"

class SakuyaNPC : virtual public Agent, public BaseAttributes<SakuyaNPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(SakuyaNPC);

	virtual inline GType getType() const { return GType::npc; }
    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/sakuya.png";}

	virtual void initStateMachine(ai::StateMachine& sm);
};

class SakuyaNPC1 : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(SakuyaMain)
};

#endif /* SakuyaNPC_hpp */
