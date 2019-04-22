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

class Rumia : public Enemy, public BaseAttributes<Rumia>, public RadialLightObject
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(Rumia);

	virtual inline string getProperName() const { return "Rumia"; }

    virtual inline SpaceFloat getRadius() const {return 0.35;}
    virtual inline SpaceFloat getMass() const {return 20.0;}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
	virtual CircleLightArea getLightSource() const;
    virtual inline string imageSpritePath() const {return "sprites/rumia.png";}
	virtual inline bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 128; }
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& fsm);
	virtual void onZeroHP();
};

class Rumia2 : public Enemy, public BaseAttributes<Rumia2>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(Rumia2);

	virtual inline string getProperName() const { return "Rumia"; }

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	virtual inline SpaceFloat getMass() const { return 20.0; }

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	virtual inline string imageSpritePath() const { return "sprites/rumia.png"; }
	virtual inline bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 128; }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& fsm);
	virtual void onZeroHP();
};

class RumiaDSD2 : public ai::Function
{
public:
	inline RumiaDSD2() {}

	static const vector<double_pair> demarcationSizeIntervals;

	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	virtual void onExit(ai::StateMachine& sm);
	FuncGetName(RumiaDSD2);
protected:
	SpaceFloat timer = 0.0;
	size_t intervalIdx = 0;
};

#endif /* Rumia_hpp */
