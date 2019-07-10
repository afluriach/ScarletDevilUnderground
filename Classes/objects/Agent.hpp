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

class Agent : public GObject
{
public:
	static constexpr SpaceFloat defaultSize = 0.35;
	static const Color4F bodyOutlineColor;
	static const Color4F shieldConeColor;
	static const float bodyOutlineWidth;

	Agent(GSpace* space, ObjectIDType id, const string& name, const SpaceVect& pos, Direction d);
	Agent(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		const string& baseAttributes,
		SpaceFloat radius,
		SpaceFloat mass
	);
	inline virtual ~Agent() {}

	bullet_attributes getBulletAttributes(shared_ptr<bullet_properties> props) const;
	object_ref<BulletImpl> bulletImplCheckSpawn(
		shared_ptr<object_params> params,
		shared_ptr<bullet_properties> props
	);

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
	AttributeMap getBaseAttributes() const;
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
	virtual SpaceFloat getRadarRadius() const { return 0.0; }
	virtual GType getRadarType() const { return GType::none; }
	virtual SpaceFloat getDefaultFovAngle() const { return 0.0; }
	virtual inline bool hasEssenceRadar() const { return false; }

	inline RadarSensor* getRadar() { return radar; }

	virtual void initializeRadar(GSpace& space);
	virtual void removePhysicsObjects();

	//physics/motor interface
	virtual GType getType() const = 0;

	//graphics interface
	virtual void initializeGraphics();
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void setAngle(SpaceFloat a);
	virtual void setDirection(Direction d);

	void resetAnimation();
	void setSprite(const string& sprite);
	//AI interface
	inline virtual string initStateMachine() { return ""; }
protected:
	void updateAgentOverlay();
	void updateAnimation();

	SpriteID agentOverlay = 0;
	AttributeSystem attributeSystem;
	string attributes;
	shared_ptr<FirePattern> firePattern;
	unordered_set<Agent*> touchTargets;
	RadarSensor* radar = nullptr;
	unique_ptr<AgentAnimationContext> animation;

	bool shieldActive = false;
	bool wasShieldActive = false;
};

class GenericAgent : public Agent
{
public:
	static const string baseAttributes;

	MapObjCons(GenericAgent);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0;  }
    virtual inline GType getRadarType() const { return GType::enemySensor;}
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
    virtual inline GType getType() const {return GType::npc;}
        
    inline string getSprite() const {return "genericAgent";}
    //inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	inline virtual string initStateMachine() { return "wander_and_flee_player"; }
};

#endif /* Agent_hpp */
