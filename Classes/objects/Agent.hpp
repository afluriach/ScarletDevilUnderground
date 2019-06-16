//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

#include "AIMixins.hpp"
#include "Attributes.hpp"

struct bullet_properties;
class FirePattern;

class Agent :
virtual public GObject,
public PatchConSprite,
public CircleBody,
public StateMachineObject, 
public RadarObject
{
public:
	static const Color4F bodyOutlineColor;
	static const Color4F shieldConeColor;
	static const float bodyOutlineWidth;

	Agent(GSpace* space, ObjectIDType id, const string& name, const SpaceVect& pos, Direction d);
	MapObjCons(Agent);
	inline virtual ~Agent() {}

	template<class ObjectCls, typename... Args>
	inline gobject_ref bulletCheckSpawn(const SpaceVect& pos, SpaceFloat angle, Args... args)
	{
		if (!isBulletObstacle(pos, ObjectCls::props.radius))
			return space->createObject(make_object_factory<ObjectCls>(
				pos,
				angle,
				object_ref<Agent>(this),
				args...
			));
		else
			return nullptr;
	}

	template<class ObjectCls>
	inline object_ref<ObjectCls> bulletImplCheckSpawn(
		const SpaceVect& pos,
		SpaceFloat angle,
		const bullet_properties* props
	){
		if (!isBulletObstacle(pos, props->radius))
			return space->createObject(make_object_factory<ObjectCls>(
				pos,
				angle,
				object_ref<Agent>(this),
				props
			));
		else
			return nullptr;
	}

	template<class ObjectCls>
	inline object_ref<ObjectCls> bulletValueImplCheckSpawn(
		const SpaceVect& pos,
		SpaceFloat angle,
		bullet_properties props
	) {
		if (!isBulletObstacle(pos, props.radius))
			return space->createObject(make_object_factory<ObjectCls>(
				pos,
				angle,
				object_ref<Agent>(this),
				props
			));
		else
			return nullptr;
	}

	void initFSM();
	void initAttributes();
	virtual void update();

	bool isBulletObstacle(SpaceVect pos, SpaceFloat radius);

	void sendAlert(Player* p);

	//replaces functionality of RadarStateMachineObject by connecting sensor callbacks
	virtual void onDetect(GObject* obj);
	virtual void onEndDetect(GObject* obj);

	virtual void onZeroHP();
	virtual void onRemove();

	//polymorphic spellcasting - for attribute cost
	virtual bool cast(shared_ptr<Spell> spell);
	virtual void updateSpells();

	//attribute interface
	virtual AttributeMap getBaseAttributes() const = 0;
	virtual inline AttributeMap getAttributeUpgrades() const { return AttributeMap(); }
	float getAttribute(Attribute id) const;
	void modifyAttribute(Attribute id, float val);
	inline AttributeSystem* getAttributeSystem() { return &attributeSystem; }
	inline FirePattern* getFirePattern() const { return firePattern.get(); }

	virtual SpaceFloat getTraction() const;

	virtual SpaceFloat getMaxSpeed() const;
	virtual SpaceFloat getMaxAcceleration() const;

	virtual float getMaxHealth() const;

	int getHealth();
	SpaceFloat getHealthRatio();
	int getStamina();
	int getMagic();
	bool consumeStamina(int val);

	void setShieldActive(bool v);
	inline bool isShieldActive() const { return shieldActive; }
	bool isShield(Bullet* b);
	float getShieldCost(SpaceVect n);
	virtual void onBulletCollide(Bullet* b);
	void onTouchAgent(Agent* other);
	void onEndTouchAgent(Agent* other);

	inline virtual void onBulletHitTarget(Bullet* bullet, Agent* target) {}
	virtual bool hit(DamageInfo damage);
	bool canApplyAttributeEffects(AttributeMap attributeEffect);
	void applyAttributeEffects(AttributeMap attributeEffects);
	virtual DamageInfo touchEffect() const;

	//sensor interface
	virtual SpaceFloat getRadarRadius() const { return 1.0; }
	virtual GType getRadarType() const { return GType::none; }
	virtual SpaceFloat getDefaultFovAngle() const { return 0.0; }

	//physics/motor interface
	virtual inline SpaceFloat getRadius() const { return 0.35; }
	virtual SpaceFloat getMass() const = 0;
	virtual GType getType() const = 0;

	//graphics interface
	virtual void initializeGraphics();
	inline string imageSpritePath() const = 0;
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//AI interface
	virtual inline void initStateMachine() {}
protected:
	void updateAgentOverlay();

	SpriteID agentOverlay = 0;
	AttributeSystem attributeSystem;
	shared_ptr<FirePattern> firePattern;
	unordered_set<Agent*> touchTargets;

	bool shieldActive = false;
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
		return AttributeMap();
	}
};

template<class C>
class AIPackage : virtual public Agent
{
public: 
	using fsmInitFunction = void(C::*)(const ValueMap& args);

	typedef unordered_map<string, fsmInitFunction> AIPackageMap;

	inline AIPackage(C* agent, const ValueMap& args, string _default) :
	agent(agent),
	args(args)
	{
		packageName = getStringOrDefault(args, "ai_package", _default);
	}

	inline virtual void initStateMachine()
	{
		auto it = C::aiPackages.find(packageName);
		if (it != C::aiPackages.end()){
			fsmInitFunction f = it->second;
			(agent->*f)(args);
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
    virtual inline GType getRadarType() const { return GType::enemySensor;}
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
	inline SpaceFloat getMass() const { return 20.0; }
    virtual inline GType getType() const {return GType::npc;}
        
    inline string imageSpritePath() const {return "sprites/"+spriteName+".png";}
    //inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual void initStateMachine();
protected:
    string spriteName;
};

#endif /* Agent_hpp */
