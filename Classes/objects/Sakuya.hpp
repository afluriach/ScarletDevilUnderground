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
	static const AttributeMap baseAttributes;

	MapObjCons(Sakuya);

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/sakuya.png";}

	virtual void initStateMachine(ai::StateMachine& sm);
};

class SakuyaMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(SakuyaMain)
};

class IllusionDash : public ai::Function {
public:
	IllusionDash(SpaceVect _target);
	IllusionDash(GSpace* space, const ValueMap& args);
	static const SpaceFloat scale;
	static const SpaceFloat opacity;
	static const SpaceFloat speed;

	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(IllusionDash)
protected:
	SpaceVect target;
};

#endif /* Sakuya_hpp */
