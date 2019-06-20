//
//  Sakuya.hpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#ifndef Sakuya_hpp
#define Sakuya_hpp

#include "Enemy.hpp"

class Sakuya :
public Enemy,
public BaseAttributes<Sakuya>
{
public:
	static const string baseAttributes;

	MapObjCons(Sakuya);

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

    inline SpaceFloat getMass() const {return 40.0;}

    inline string getSprite() const {return "sakuya";}

	virtual void initStateMachine();
};

class SakuyaMain : public ai::Function {
public:
	inline SakuyaMain(ai::StateMachine* fsm) : ai::Function(fsm) {}

	virtual void onEnter();
	virtual ai::update_return update();
	FuncGetName(SakuyaMain)
};

class IllusionDash : public ai::Function {
public:
	inline IllusionDash(ai::StateMachine* fsm) : ai::Function(fsm) {}

	IllusionDash(ai::StateMachine* fsm, SpaceVect _target);
	IllusionDash(ai::StateMachine* fsm, const ValueMap& args);
	static const SpaceFloat scale;
	static const SpaceFloat opacity;
	static const SpaceFloat speed;

	virtual void onEnter();
	virtual ai::update_return update();
	FuncGetName(IllusionDash)
protected:
	SpaceVect target;
};

#endif /* Sakuya_hpp */
