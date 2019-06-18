//
//  Stalker.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Stalker_hpp
#define Stalker_hpp

#include "Enemy.hpp"

class Stalker : public Enemy, public BaseAttributes<Stalker>
{
public:
	static const AttributeMap baseAttributes;

	Stalker(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline bool hasEssenceRadar() const { return true; }
    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getMass() const {return 40.0;}

	virtual inline string getSprite() const {return "stalker";}

	virtual void initStateMachine();
	virtual void teleport(SpaceVect pos);

	virtual void update();
protected:
	SpaceVect prev_pos;
};

class StalkerMain : public ai::Function {
public:
	inline StalkerMain(ai::StateMachine* fsm) : ai::Function(fsm) {}

	virtual void onEnter();
	virtual ai::update_return update();
	FuncGetName(StalkerMain)
};

#endif /* Stalker_hpp */
