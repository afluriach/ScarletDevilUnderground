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
public RegisterUpdate<Agent>
{
public:
	MapObjCons(Agent);
	inline virtual ~Agent() {}

	void initFSM();
	void initAttributes();
	void update();

	//replaces functionality of RadarStateMachineObject by connecting sensor callbacks
	virtual void onDetect(GObject* obj);
	virtual void onEndDetect(GObject* obj);

	virtual void onZeroHP();

	//attribute interface
	virtual AttributeMap getBaseAttributes() const = 0;
	virtual inline AttributeMap getAttributeUpgrades() const { return AttributeMap(); }
	float getAttribute(Attribute id) const;
	void modifyAttribute(Attribute id, float val);
	float _getAttribute(int id) const;
	void _modifyAttribute(int id, float val);
	inline const AttributeSystem& getAttributeSystem() const { return attributeSystem; }

	virtual SpaceFloat getTraction() const;

	virtual SpaceFloat getMaxSpeed() const;
	virtual SpaceFloat getMaxAcceleration() const;

	virtual float getMaxHealth() const;
	virtual float getMaxPower() const;

	int getHealth();
	SpaceFloat getHealthRatio();
	int getPower();
	int getMagic();
	bool consumePower(int val);

	virtual void hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect);
	bool canApplyAttributeEffects(AttributeMap attributeEffect);
	void applyAttributeEffects(AttributeMap attributeEffects);

	//sensor interface
	virtual SpaceFloat getRadarRadius() const { return 1.0; }
	virtual GType getRadarType() const { return GType::none; }
	virtual SpaceFloat getDefaultFovAngle() const { return 0.0; }

	//physics/motor interface
	virtual inline SpaceFloat getRadius() const { return 0.35; }
	virtual SpaceFloat getMass() const = 0;
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

template<class C>
class AIPackage : virtual public Agent
{
public: 
	using fsmInitFunction = void(C::*)(ai::StateMachine&, const ValueMap& args);

	typedef map<string, fsmInitFunction> AIPackageMap;

	inline AIPackage(C* agent, const ValueMap& args, string _default) :
	agent(agent),
	args(args)
	{
		packageName = getStringOrDefault(args, "ai_package", _default);
	}

	inline virtual void initStateMachine(ai::StateMachine& sm)
	{
		auto it = C::aiPackages.find(packageName);
		if (it != C::aiPackages.end()){
			fsmInitFunction f = it->second;
			(agent->*f)(sm, args);
		}
		args.clear();
	}
protected:
	string packageName;
	C* agent;
	ValueMap args;
};

template<class C>
class AttributesPackage : virtual public Agent
{
public:
	inline AttributesPackage(C* agent, const ValueMap& args) :
	agent(agent),
	attributes(C::baseAttributes)
	{
		string packageName = getStringOrDefault(args, "attributes_package", "");

		if (!packageName.empty())
		{
			auto it = C::attributePackages.find(packageName);
			if (it != C::attributePackages.end()) {
				attributes = it->second;
			}
		}
	}

	inline virtual AttributeMap getBaseAttributes() const {
		return attributes;
	}

protected:
	AttributeMap attributes;
	C* agent;
};

class GenericAgent : virtual public Agent, public BaseAttributes<GenericAgent>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(GenericAgent);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0;  }
    virtual inline GType getRadarType() const { return GType::playerSensor;}
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
	inline SpaceFloat getMass() const { return 20.0; }
    virtual inline GType getType() const {return GType::npc;}
        
    inline string imageSpritePath() const {return "sprites/"+spriteName+".png";}
    //inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine(ai::StateMachine& sm);
protected:
    string spriteName;
};

#endif /* Agent_hpp */
