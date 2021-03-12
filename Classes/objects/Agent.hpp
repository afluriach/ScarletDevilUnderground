//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

class agent_properties : public object_properties
{
public:
	inline agent_properties() {}
	virtual inline ~agent_properties() {}

	AttributeMap attributes;

	string ai_package;

	const SpellDesc* attack;
	list<const SpellDesc*> spellInventory;
	list<effect_entry> effects;

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
	void applyEffects();
	AttributeMap getBaseAttributes() const;
	void checkInitScriptObject();
	virtual void init();
	virtual void update();

	inline virtual int getLevel() const { return level; }

	void sendAlert(Player* p);

	void onDetect(GObject* obj);
	void onEndDetect(GObject* obj);

	void onDetectEnemy(Agent* enemy);
	void onEndDetectEnemy(Agent* enemy);
	void onDetectBullet(Bullet* bullet);
	void onDetectBomb(Bomb* bomb);

	virtual void onZeroHP();

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

	bool canPlaceBomb(SpaceVect pos);
	void setShieldActive(bool v);
	inline bool isShieldActive() const { return shieldActive; }
	bool isShield(Bullet* b);
	float getShieldCost(SpaceVect n);
	virtual void onBulletCollide(Bullet* b, SpaceVect n);
	void onTouchAgent(Agent* other);
	void onEndTouchAgent(Agent* other);
	bool isEnemy(Agent* other);
	bool isEnemyBullet(Bullet* other);

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

	virtual void setAngle(SpaceFloat a);
	virtual void setDirection(Direction d);

	void resetAnimation();
	void setSprite(shared_ptr<sprite_properties> sprite);
protected:
	void updateAnimation();

	string ai_package;
	int level = 0;

	//equips
	local_shared_ptr<FirePattern> firePattern;
	local_shared_ptr<bomb_properties> crntBomb;
	const SpellDesc* powerAttack;

	RadarSensor* radar = nullptr;
	AttributeSystem* attributeSystem = nullptr;
	local_shared_ptr<agent_properties> props;
	unique_ptr<AgentAnimationContext> animation;

	bool shieldActive = false;
	bool wasShieldActive = false;
};

#endif /* Agent_hpp */
