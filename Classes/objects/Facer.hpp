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
	static const string properName;

	MapObjCons(Facer);

	virtual bool hit(DamageInfo damage);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string getSprite() const {return "facer";}
 
	virtual void initStateMachine();
};

class FacerMain : public ai::Function {
public:
	inline FacerMain(ai::StateMachine* fsm) : ai::Function(fsm) {}

	virtual void onEnter();
	virtual ai::update_return update();
	FuncGetName(FacerMain)
protected:
	gobject_ref target = nullptr;
};

#endif /* Facer_hpp */
