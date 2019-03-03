//
//  Bat.hpp
//  Koumachika
//
//  Created by Toni on 2/20/19.
//
//

#ifndef Bat_hpp
#define Bat_hpp

#include "Enemy.hpp"

class Bat : public Enemy, public BaseAttributes<Bat>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(Bat);

	inline virtual AttributeMap touchEffect() {
		return AttributeSystem::getAttributeMap(Attribute::hp, -5.0f);
	}

	virtual inline SpaceFloat getRadarRadius() const { return 5.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    inline SpaceFloat getMass() const {return 10.0;}

    inline string imageSpritePath() const {return "sprites/bat.png";}
 
	virtual void initStateMachine(ai::StateMachine& sm);
};

class BatMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(BatMain)

	void pushQuadraticStrike(ai::StateMachine& sm);
protected:
	gobject_ref target = nullptr;
};

#endif /* Bat_hpp */
