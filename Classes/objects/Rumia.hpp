//
//  Rumia.hpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#ifndef Rumia_hpp
#define Rumia_hpp

#include "Enemy.hpp"

class Rumia : public Enemy
{
public:
	MapObjCons(Rumia);

    virtual inline SpaceFloat getMass() const {return 20.0;}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
	virtual shared_ptr<LightArea> getLightSource() const;
    virtual inline string getSprite() const {return "rumia";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
};

class Rumia1 : public Rumia, public BaseAttributes<Rumia1>
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(Rumia1);

	virtual void initStateMachine();
	virtual void onZeroHP();
};

class Rumia2 : public Rumia, public BaseAttributes<Rumia2>
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(Rumia2);

	virtual void initStateMachine();
	virtual void onZeroHP();
};

class RumiaMain1 : public ai::Function
{
public:
	static const SpaceFloat dsdDistMargin;
	static const SpaceFloat dsdLength;
	static const SpaceFloat dsdCooldown;
	static const float dsdCost;

	RumiaMain1(ai::StateMachine* fsm, gobject_ref target);

	virtual void onEnter();
	virtual void onReturn();
	virtual ai::update_return update();
	virtual void onExit();
	FuncGetName(RumiaMain1);
protected:
	gobject_ref target;
	SpaceFloat dsdTimer = 0.0;
	size_t intervalIdx = 0;
};

class RumiaDSD2 : public ai::Function
{
public:
	inline RumiaDSD2(ai::StateMachine* fsm) : ai::Function(fsm) {}

	static const vector<double_pair> demarcationSizeIntervals;

	virtual void onEnter();
	virtual ai::update_return update();
	virtual void onExit();
	FuncGetName(RumiaDSD2);
protected:
	SpaceFloat timer = 0.0;
	size_t intervalIdx = 0;
};

#endif /* Rumia_hpp */
