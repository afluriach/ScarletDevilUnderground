//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

#include "AI.hpp"
#include "AIMixins.hpp"
#include "Attributes.hpp"
#include "GObject.hpp"

class Agent :
virtual public GObject,
public PatchConSprite,
public CircleBody,
public StateMachineObject, 
public RadarObject,
public RegisterInit<Agent>,
public RegisterUpdate<Agent>
{
public:
	MapObjCons(Agent);

	void init();
	void update();

	//replaces functionality of RadarStateMachineObject by connecting sensor callbacks
	virtual void onDetect(GObject* obj);
	virtual void onEndDetect(GObject* obj);

	virtual void onZeroHP();

	//attribute interface
	virtual AttributeMap getBaseAttributes() const = 0;
	float getAttribute(Attribute id) const;

	virtual float getMaxSpeed() const;
	virtual float getMaxAcceleration() const;

	virtual float getMaxHealth() const;
	virtual float getMaxPower() const;

	int getHealth();
	int getPower();
	bool consumePower(int val);

	virtual void hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect);

	//sensor interface
	virtual float getRadarRadius() const { return 1.0f; }
	virtual GType getRadarType() const { return GType::none; }
	virtual float getDefaultFovAngle() const { return 0.0f; }

	//physics/motor interface
	virtual inline float getRadius() const { return 0.35f; }
	virtual float getMass() const = 0;
	virtual GType getType() const = 0;

	//graphics interface
	inline string imageSpritePath() const = 0;
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//AI interface
	virtual inline void initStateMachine(ai::StateMachine& sm) {}
protected:
	AttributeSystem attributeSystem;
};

template<class T>
class BaseAttributes : virtual public Agent
{
public:
	inline BaseAttributes() {}

	inline virtual AttributeMap getBaseAttributes() const {
		return T::baseAttributes;
	}
};

class NoAttributes : virtual public Agent
{
public:
	inline NoAttributes() {}

	inline virtual AttributeMap getBaseAttributes() const {
		return {};
		return AttributeMap();
	}
};

class GenericAgent : virtual public Agent, public BaseAttributes<GenericAgent>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(GenericAgent);

    virtual inline float getRadarRadius() const {return 3.0f;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline float getDefaultFovAngle() const {return 0.0f;}
    
    //virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 20.0f;}
    virtual inline GType getType() const {return GType::npc;}
        
    inline string imageSpritePath() const {return "sprites/"+spriteName+".png";}
    //inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& sm);
protected:
    string spriteName;
};

#endif /* Agent_hpp */
