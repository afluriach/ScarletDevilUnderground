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
	static const string properName;

	MapObjCons(Bat);

	virtual inline SpaceFloat getRadarRadius() const { return 5.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    virtual inline SpaceFloat getMass() const {return 10.0;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

    virtual inline string getSprite() const { return "bat";}
 
	virtual void initStateMachine();
};

class BatMain : public ai::Function {
public:
	inline BatMain(ai::StateMachine* fsm) : ai::Function(fsm) {}

	virtual void onEnter();
	virtual ai::update_return update();
	FuncGetName(BatMain)
protected:
	gobject_ref target = nullptr;
};

#endif /* Bat_hpp */
