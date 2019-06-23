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
#include "Bullet.hpp"

struct bullet_properties;
class FirePattern;

#define AgentMapForward(x) Agent(space,id,args,x)

class Agent :
virtual public GObject,
public PatchConSprite,
public CircleBody,
public RadarObject
{
public:
	static constexpr SpaceFloat defaultSize = 0.35;
	static const Color4F bodyOutlineColor;
	static const Color4F shieldConeColor;
	static const float bodyOutlineWidth;

	Agent(GSpace* space, ObjectIDType id, const string& name, const SpaceVect& pos, Direction d);
	Agent(GSpace* space, ObjectIDType id, const ValueMap& args, SpaceFloat radius = defaultSize);
	inline virtual ~Agent() {}

	bullet_attributes getBulletAttributes(shared_ptr<bullet_properties> props) const;

	template<class ObjectCls, typename... Args>
	inline gobject_ref bulletCheckSpawn(const SpaceVect& pos, SpaceFloat angle, Args... args)
	{
		auto props = app::getBullet(ObjectCls::props);
		SpaceFloat radius = props->radius;

		if (!isBulletObstacle(pos, radius))
			return space->createObject<ObjectCls>(
				pos,
				angle,
				getBulletAttributes(props),
				args...
			);
		else
			return nullptr;
	}

	template<class ObjectCls>
	inline object_ref<ObjectCls> bulletImplCheckSpawn(
		const SpaceVect& pos,
		SpaceFloat angle,
		shared_ptr<bullet_properties> props
	){
		if (!isBulletObstacle(pos, props->radius))
			return space->createObject<ObjectCls>(
				pos,
				angle,
				getBulletAttributes(props),
				props
			);
		else
			return nullptr;
	}

	void initFSM();
	void initAttributes();
	virtual void init();
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
	virtual inline AttributeMap getBaseAttributes() const { return AttributeMap(); }
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
	virtual SpaceFloat getMass() const = 0;
	virtual GType getType() const = 0;

	//graphics interface
	virtual void initializeGraphics();
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//AI interface
	inline virtual string initStateMachine() { return ""; }
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
		return app::getAttributes(T::baseAttributes);
	}
};

class GenericAgent : virtual public Agent, public BaseAttributes<GenericAgent>
{
public:
	static const string baseAttributes;

	MapObjCons(GenericAgent);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0;  }
    virtual inline GType getRadarType() const { return GType::enemySensor;}
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
	inline SpaceFloat getMass() const { return 20.0; }
    virtual inline GType getType() const {return GType::npc;}
        
    inline string getSprite() const {return "genericAgent";}
    //inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	inline virtual string initStateMachine() { return "wander_and_flee_player"; }
};

#endif /* Agent_hpp */
