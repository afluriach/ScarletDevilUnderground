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

	MapObjCons(Facer);

	virtual bool hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/tewi.png";}
 
	virtual void initStateMachine(ai::StateMachine& sm);
};

class FacerMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(FacerMain)
protected:
	gobject_ref target = nullptr;
};

#endif /* Facer_hpp */
