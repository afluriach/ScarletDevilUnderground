//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

class Inventory;

enum class agent_state
{
    none,
    
    blocking,
    powerAttack,
    sprinting,
    sprintRecovery,
    knockback,
};

struct power_attack_data{
    local_shared_ptr<Spell> attack;
};

struct sprint_data{
    SpaceVect sprintDirection;
};

struct knockback_data{
	float accumulator = 0.0f;
};

typedef variant<power_attack_data, sprint_data, knockback_data> state_data;

class Agent : public GObject
{
public:
    friend class PlayerControl;

	static constexpr SpaceFloat defaultSize = 1.0 / 3.0;
	static const Color4F bodyOutlineColor;
	static const Color4F shieldConeColor;
	static const float bodyOutlineWidth;
    static const float bombSpawnDistance;
    static const float minKnockbackTime;
    static const float maxKnockbackTime;

	Agent(
		GSpace* space,
		ObjectIDType id,
		GType type,
		const object_params& params,
		local_shared_ptr<agent_properties> props
	);
	virtual ~Agent();

	void initAttributes();
	void applyEffects();
	AttributeMap getBaseAttributes() const;
	virtual void init();
	virtual void update();

	virtual int getLevel() const;

	void sendAlert(Player* p);

	void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	void onDetectEnemy(Agent* enemy);
	void onEndDetectEnemy(Agent* enemy);
	void onDetectBullet(Bullet* bullet);
	void onDetectBomb(Bomb* bomb);

	virtual void onZeroHP();

    void updateCombo();
	virtual bool applyInitialSpellCost(const spell_cost& cost);
	virtual bool applyOngoingSpellCost(const spell_cost& cost);
    bool canApplySpellCost(const spell_cost& cost);

	virtual bullet_attributes getBulletAttributes(local_shared_ptr<bullet_properties> props) const;

	//attribute interface
	inline void increment(Attribute attr) { attributeSystem->increment(attr); }
	inline void decrement(Attribute attr) { attributeSystem->decrement(attr); }
	inline bool isActive(Attribute attr) const { return attributeSystem->isNonzero(attr); }

	virtual inline AttributeMap getAttributeUpgrades() const { return AttributeMap(); }
	float get(Attribute id) const;
    void setAttribute(Attribute id, float val) const;
    void setAttribute(Attribute id, Attribute val) const;
	inline float operator[](Attribute attr) const { return (*attributeSystem)[attr]; }
	void modifyAttribute(Attribute id, float val);
	void modifyAttribute(Attribute mod, Attribute addend);
	void modifyAttribute(Attribute mod, Attribute addend, float scale);
	bool consume(Attribute attr, float val);
	inline AttributeSystem* getAttributeSystem() { return attributeSystem; }
 
    //inventory / equip interface
    inline local_shared_ptr<bomb_properties> getBomb() const {return crntBomb; }
	inline FirePattern* getFirePattern() const { return firePattern.get(); }
	bool setFirePattern(string firePattern);
    bool setFirePattern(local_shared_ptr<FirePattern> firePattern);
    Inventory* getInventory() const { return inventory.get(); }

	inline virtual bool isInvisible() const { return attributeSystem->isNonzero(Attribute::invisibility); }

	virtual SpaceFloat getTraction() const;
	virtual SpaceFloat getMaxSpeed() const;
	virtual SpaceFloat getMaxAcceleration() const;
    SpaceFloat getSpeedMultiplier() const;
    SpaceFloat getAccelMultiplier() const;

	bool canPlaceBomb(SpaceVect pos);
	void setShieldActive(bool v);
	bool isShield(Bullet* b);
	float getShieldCost(SpaceVect n);
	virtual void onBulletCollide(Bullet* b, SpaceVect n);
	void onTouchAgent(Agent* other);
	void onEndTouchAgent(Agent* other);
	bool isEnemy(Agent* other);
	bool isEnemyBullet(Bullet* other);
	void applyKnockback(SpaceVect f);

	inline virtual void onBulletHitTarget(Bullet* bullet, Agent* target) {}
	virtual bool hit(DamageInfo damage, SpaceVect n);
	void applyAttributeEffects(AttributeMap attributeEffects);
	virtual DamageInfo touchEffect() const;

	//sensor interface
	bool hasEssenceRadar() const;
	SpaceFloat getRadarRadius() const;
	SpaceFloat getDefaultFovAngle() const;

	inline RadarSensor* getRadar() { return radar; }
	SpaceFloat getSensedObjectDistance(GType type) const;

	void initializeRadar();
	virtual void removePhysicsObjects();

	//graphics interface
	virtual void initializeGraphics();
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

    //action interface
    bool fire();
    bool aimAtTarget(gobject_ref target);
    bool canSprint();
    void sprint(SpaceVect direction);
    bool canBlock();
    void block();
    void endBlock();
    bool hasPowerAttack();
    bool powerAttackAvailable();
    bool doPowerAttack(const SpellDesc* p);
    bool doPowerAttack();
    bool isBombAvailable();
    bool throwBomb(local_shared_ptr<bomb_properties> bomb, SpaceFloat speedRatio);
    
    void applyDesiredMovement(SpaceVect direction);
    
    bool canCast(const SpellDesc* desc);
    
    void selectNextSpell();
    void selectPrevSpell();
    void selectNextFirePattern();
    void selectPrevFirePattern();
    
    void setState(agent_state newState);
    void updateState();
    void endState();

    bool isShieldActive();
    bool isSprintActive();

	virtual void setAngle(SpaceFloat a);
	virtual void setDirection(Direction d);

	void resetAnimation();
	void setSprite(shared_ptr<sprite_properties> sprite);
protected:
	void updateAnimation();
	
	void _updateSprinting();
	void _updateSprintRecovery();
	void _updatePowerAttack();
	void _updateKnockback();
	
	void _endPowerAttack();
 
    agent_state crntState = agent_state::none;
    SpaceFloat timeInState = 0.0;
    state_data stateData;

	//equips
    unique_ptr<Inventory> inventory;
   
	local_shared_ptr<FirePattern> firePattern;
	local_shared_ptr<bomb_properties> crntBomb;
	const SpellDesc* powerAttack = nullptr;
    const SpellDesc* equippedSpell = nullptr;

	RadarSensor* radar = nullptr;
	AttributeSystem* attributeSystem = nullptr;
	local_shared_ptr<agent_properties> props;
	unique_ptr<AgentAnimationContext> animation;
    SpriteID agentOverlay = 0;
};

#endif /* Agent_hpp */
