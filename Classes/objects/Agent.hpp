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
	Agent(const ValueMap& args);

	void init();
	void update();

	//replaces functionality of RadarStateMachineObject by connecting sensor callbacks
	inline virtual void onDetect(GObject* obj) {
		fsm.onDetect(obj);
		RadarObject::onDetect(obj);
	}

	inline virtual void onEndDetect(GObject* obj) {
		fsm.onEndDetect(obj);
		RadarObject::onEndDetect(obj);
	}

	//attribute interface
	virtual AttributeMap getBaseAttributes() const = 0;
	float getCrntAttribute(Attribute id);

	virtual float getMaxSpeed() const;
	virtual float getMaxAcceleration() const;

	virtual float getMaxHealth() const;
	virtual float getMaxPower() const;

	virtual void hit(int damage, shared_ptr<MagicEffect> effect);

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

	int health;
	int power;
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
	}
};

class GenericAgent : virtual public Agent, public BaseAttributes<GenericAgent>
{
public:
	static const AttributeMap baseAttributes;

	GenericAgent(const ValueMap& args);
    
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
