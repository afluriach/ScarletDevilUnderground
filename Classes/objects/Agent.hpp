//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

class AgentAnimationContext;
struct bullet_properties;
class FirePattern;
class RadarSensor;
struct spell_cost;

class agent_properties : public object_properties
{
public:
	inline agent_properties() {}
	virtual inline ~agent_properties() {}

	AttributeMap attributes;

	string ai_package;
	string effects;

	SpaceFloat viewRange = 0.0;
	SpaceFloat viewAngle = 0.0;

	bool detectEssence = false;
	bool isFlying = false;

	inline virtual type_index getType() const { return typeid(*this); }
};

class Agent : public GObject
{
public:
	static constexpr SpaceFloat defaultSize = 0.35;
	static const Color4F bodyOutlineColor;
	static const Color4F shieldConeColor;
	static const float bodyOutlineWidth;

	static bool conditionalLoad(GSpace* space, const object_params& params, local_shared_ptr<agent_properties> props);

	Agent(
		GSpace* space,
		ObjectIDType id,
		GType type,
		const object_params& params,
		local_shared_ptr<agent_properties> props
	);
	virtual ~Agent();

	void initFSM();
	void initAttributes();
	AttributeMap getBaseAttributes() const;
	void checkInitScriptObject();
	virtual void init();
	virtual void update();

	inline int getLevel() const { return level; }

	void sendAlert(Player* p);

	//replaces functionality of RadarStateMachineObject by connecting sensor callbacks
	virtual void onDetect(GObject* obj);
	virtual void onEndDetect(GObject* obj);

	virtual void onZeroHP();
	virtual void onRemove();

	virtual bool applyInitialSpellCost(const spell_cost& cost);
	virtual bool applyOngoingSpellCost(const spell_cost& cost);

	virtual bullet_attributes getBulletAttributes(local_shared_ptr<bullet_properties> props) const;

	//attribute interface
	inline void increment(Attribute attr) { attributeSystem->increment(attr); }
	inline void decrement(Attribute attr) { attributeSystem->decrement(attr); }
	inline bool isActive(Attribute attr) const { return attributeSystem->isNonzero(attr); }

	virtual inline AttributeMap getAttributeUpgrades() const { return AttributeMap(); }
	float get(Attribute id) const;
	inline float operator[](Attribute attr) const { return (*attributeSystem)[attr]; }
	void modifyAttribute(Attribute id, float val);
	void modifyAttribute(Attribute mod, Attribute addend);
	void modifyAttribute(Attribute mod, Attribute addend, float scale);
	bool consume(Attribute attr, float val);
	inline AttributeSystem* getAttributeSystem() { return attributeSystem; }
	inline FirePattern* getFirePattern() const { return firePattern.get(); }
	bool setFirePattern(string firePattern);

	inline virtual bool isInvisible() const { return attributeSystem->isNonzero(Attribute::invisibility); }

	virtual SpaceFloat getTraction() const;

	virtual SpaceFloat getMaxSpeed() const;
	virtual SpaceFloat getMaxAcceleration() const;

	void setShieldActive(bool v);
	inline bool isShieldActive() const { return shieldActive; }
	bool isShield(Bullet* b);
	float getShieldCost(SpaceVect n);
	virtual void onBulletCollide(Bullet* b, SpaceVect n);
	void onTouchAgent(Agent* other);
	void onEndTouchAgent(Agent* other);

	inline virtual void onBulletHitTarget(Bullet* bullet, Agent* target) {}
	virtual bool hit(DamageInfo damage, SpaceVect n);
	void applyAttributeEffects(AttributeMap attributeEffects);
	virtual DamageInfo touchEffect() const;

	//sensor interface
	bool hasEssenceRadar() const;
	SpaceFloat getRadarRadius() const;
	SpaceFloat getDefaultFovAngle() const;

	inline RadarSensor* getRadar() { return radar; }

	void initializeRadar();
	virtual void removePhysicsObjects();

	//graphics interface
	virtual void initializeGraphics();
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void setAngle(SpaceFloat a);
	virtual void setDirection(Direction d);

	void resetAnimation();
	void setSprite(shared_ptr<sprite_properties> sprite);
protected:
	void updateAgentOverlay();
	void updateAnimation();

	string ai_package;
	int level = 0;
	SpriteID agentOverlay = 0;

	local_shared_ptr<FirePattern> firePattern;
	RadarSensor* radar = nullptr;
	AttributeSystem* attributeSystem = nullptr;
	local_shared_ptr<agent_properties> props;
	unique_ptr<AgentAnimationContext> animation;

	bool shieldActive = false;
	bool wasShieldActive = false;
};

#endif /* Agent_hpp */
