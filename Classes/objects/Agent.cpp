//
//  Agent.cpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#include "Prefix.h"

//#include "Agent.hpp"
#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "Bomb.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "FirePattern.hpp"
#include "FirePatternImpl.hpp"
#include "FloorSegment.hpp"
#include "GAnimation.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GraphicsNodes.hpp"
#include "HUD.hpp"
#include "Inventory.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "RadarSensor.hpp"
#include "SpellDescriptor.hpp"
#include "spell_types.hpp"

const Color4F Agent::bodyOutlineColor = hsva4F(270.0f, 0.2f, 0.7f, 0.667f);
const Color4F Agent::shieldConeColor = Color4F(.37f, .56f, .57f, 0.333f);
const float Agent::bodyOutlineWidth = 4.0f;
const float Agent::bombSpawnDistance = 1.0f;
const float Agent::minKnockbackTime = 0.25f;
const float Agent::maxKnockbackTime = 1.0f;

Agent::Agent(
	GSpace* space,
	ObjectIDType id,
	GType type,
	const object_params& params,
	local_shared_ptr<agent_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(
			type,
			PhysicsLayers::onGround,
			props->mass,
			false,
			!props->isFlying
		),
		props
	),
	props(props)
{
    inventory = make_unique<Inventory>();
}

Agent::~Agent()
{
	allocator_delete(attributeSystem);
}

bullet_attributes Agent::getBulletAttributes(local_shared_ptr<bullet_properties> props) const
{
	bullet_attributes result;

	result.caster = gobject_ref(this);
	result.startRoom = crntRoom;
	result.casterVelocity = getVel();
	result.type = getType() == GType::player ? GType::playerBullet : GType::enemyBullet;
	result.attackDamage = get(Attribute::attack);
	result.bulletSpeed = get(Attribute::bulletSpeed);

	return result;
}

bool Agent::hasEssenceRadar() const {
	return props->detectEssence;
}

SpaceFloat Agent::getRadarRadius() const {
	return props->viewRange;
}

SpaceFloat Agent::getDefaultFovAngle() const {
	return props->viewAngle;
}

void Agent::initAttributes()
{
	attributeSystem = allocator_new<AttributeSystem>(getBaseAttributes());
	applyAttributeEffects(getAttributeUpgrades());
}

void Agent::applyEffects()
{
	for (auto entry : props->effects) {
		applyMagicEffect(entry.first, entry.second);
	}
}

void Agent::init()
{
	GObject::init();

	initAttributes();
	applyEffects();
	initializeRadar();
 
 	agentOverlay = space->createSprite(
		&graphics_context::createAgentBodyShader,
		GraphicsLayer::agentOverlay,
		bodyOutlineColor,
		shieldConeColor,
		to_float(getRadius()*app::pixelsPerTile),
		to_float(Player::grazeRadius*app::pixelsPerTile),
		bodyOutlineWidth,
		getInitialCenterPix()
	);
	space->graphicsNodeAction(&Node::setVisible, agentOverlay, false);
 
}

void Agent::update()
{
	GObject::update();

	if ( (*this)[Attribute::hp] <= 0.0f && (*this)[Attribute::maxHP] >  0.0f) {
		onZeroHP();
	}
 
    updateCombo();
    updateState();

	if (firePattern) firePattern->update();
	attributeSystem->update(this);
	updateAnimation();
 
    if(crntState == agent_state::blocking){
        space->addGraphicsAction(
			&graphics_context::setSpritePosition,
			agentOverlay,
			toCocos(getPos()*app::pixelsPerTile)
		);
		space->graphicsNodeAction(
			&Node::setRotation,
			agentOverlay,
			toCocosAngle(getAngle())
		);
    }
    
}

int Agent::getLevel() const {
    return (*this)[Attribute::level];
}

void Agent::sendAlert(Player* p)
{
    runMethodIfAvailable("roomAlert", p);
}

void Agent::onDetect(GObject* obj)
{
	Agent* other = dynamic_cast<Agent*>(obj);
	Bomb* bomb = dynamic_cast<Bomb*>(obj);
	Bullet* bullet = dynamic_cast<Bullet*>(obj);

	switch (obj->getType())
	{
	case GType::player:
	case GType::enemy:
	case GType::npc:
		if (other && isEnemy(other)) {
			onDetectEnemy(other);
		}
		break;
	case GType::bomb:
		if (other) {
			onDetectBomb(bomb);
		}
		break;
	case GType::enemyBullet:
	case GType::playerBullet:
		if (bullet && isEnemyBullet(bullet)) {
			onDetectBullet(bullet);
		}
	}
}

void Agent::onEndDetect(GObject* obj)
{
	Agent* other = dynamic_cast<Agent*>(obj);

	if (other && isEnemy(other)) {
		onEndDetectEnemy(other);
	}
}

void Agent::onDetectEnemy(Agent* enemy)
{
    runMethodIfAvailable("onDetectEnemy", enemy);
}

void Agent::onEndDetectEnemy(Agent* enemy)
{
    runMethodIfAvailable("onEndDetectEnemy", enemy);
}

void Agent::onDetectBomb(Bomb* bomb)
{
    runMethodIfAvailable("onDetectBomb", bomb);
}

void Agent::onDetectBullet(Bullet* bullet)
{
    runMethodIfAvailable("onDetectBullet", bullet);
}

void Agent::onZeroHP()
{
    runMethodIfAvailable("onZeroHP");
}

void Agent::updateCombo()
{
    if( (*this)[Attribute::maxCombo] <= 0.0f )
        return;

    bool isComboFull = (*this)[Attribute::combo] >= (*this)[Attribute::maxCombo];
    bool isComboEmpty = (*this)[Attribute::combo] == 0.0f;
    bool isComboActive = isActive(Attribute::comboLevel);

	if ( isComboFull && !isComboActive) {
        setAttribute(Attribute::comboLevel, 1.0f);
		modifyAttribute(Attribute::attack, 0.25f);
		space->addGraphicsAction(&graphics_context::runSpriteAction, spriteID, comboFlickerTintAction().generator);
	}
	else if (!isComboEmpty && isComboActive) {
        setAttribute(Attribute::comboLevel, 0.0f);
		modifyAttribute(Attribute::attack, -0.25f);
		space->addGraphicsAction(
			&graphics_context::stopSpriteAction,
			spriteID,
			cocos_action_tag::combo_mode_flicker
		);
		space->graphicsNodeAction(&Node::setColor, spriteID, Color3B::WHITE);
	}
}

bool Agent::applyInitialSpellCost(const spell_cost& cost)
{
	float mpCost = cost.initial_mp;
	float staminaCost = cost.initial_stamina;

	if (mpCost > (*this)[Attribute::mp] || staminaCost > (*this)[Attribute::stamina])
		return false;

	modifyAttribute(Attribute::mp, -mpCost);
	modifyAttribute(Attribute::stamina, -staminaCost);

	return true;
}

bool Agent::applyOngoingSpellCost(const spell_cost& cost)
{
	float mpCost = cost.ongoing_mp * app::params.secondsPerFrame;
	float staminaCost = cost.ongoing_stamina * app::params.secondsPerFrame;

	if (mpCost > (*this)[Attribute::mp] || staminaCost > (*this)[Attribute::stamina]) {
		return false;
	}

	modifyAttribute(Attribute::stamina, -staminaCost);
	modifyAttribute(Attribute::mp, -mpCost);

	return true;
}

bool Agent::canApplySpellCost(const spell_cost& cost)
{
    return
        (*this)[Attribute::mp] >= cost.initial_mp &&
        (*this)[Attribute::stamina] >= cost.initial_stamina
    ;
}

AttributeMap Agent::getBaseAttributes() const
{
	return props->attributes;
}

float Agent::get(Attribute id) const
{
	return (*this)[id];
}

void Agent::setAttribute(Attribute id, float val) const
{
    attributeSystem->set(id, val);
}

void Agent::setAttribute(Attribute id, Attribute val) const
{
    attributeSystem->set(id, val);
}

void Agent::modifyAttribute(Attribute id, float val)
{
	attributeSystem->modifyAttribute(id, val);
}

void Agent::modifyAttribute(Attribute mod, Attribute addend)
{
	modifyAttribute(mod, addend, 1.0f);
}

void Agent::modifyAttribute(Attribute mod, Attribute addend, float scale)
{
	attributeSystem->modifyAttribute(mod, addend, scale);
}

bool Agent::consume(Attribute attr, float val)
{
	if ((*this)[attr] >= val) {
		modifyAttribute(attr, -val);
		return true;
	}
	return false;
}

bool Agent::setFirePattern(string firePattern)
{
	auto props = app::getFirePattern(firePattern);

	if (props) {
		this->firePattern = make_local_shared<FirePatternImpl>(this, props);
		return true;
	}
	else{
		auto it = FirePattern::playerFirePatterns.find(firePattern);
		if (it != FirePattern::playerFirePatterns.end()) {
			this->firePattern = it->second(this);
			return true;
		}
		else {
			log1("Unknown fire pattern: %s", firePattern);
		}
	}

	return false;
}

bool Agent::setFirePattern(local_shared_ptr<FirePattern> firePattern)
{
    this->firePattern = firePattern;
    return true;
}

SpaceFloat Agent::getTraction() const
{
	if (get(Attribute::iceSensitivity) >= 1.0f) {
		return GObject::getTraction();
	}
	else
	{
		SpaceFloat traction = GObject::getTraction();

		traction += (1.0 - traction) * (1.0 - get(Attribute::iceSensitivity));

		return traction;
	}
}

SpaceFloat Agent::getMaxSpeed() const
{
	return (*this)[Attribute::maxSpeed];
}

SpaceFloat Agent::getMaxAcceleration() const
{
	return (*this)[Attribute::maxAcceleration];
}

SpaceFloat Agent::getSpeedMultiplier() const
{
	if (crntState == agent_state::sprinting) {
		return (*this)[Attribute::sprintSpeedRatio];
	}
	else if (crntState == agent_state::sprintRecovery) {
		return 0.0;
	}
	else if (crntState == agent_state::blocking) {
		return (*this)[Attribute::blockSpeedRatio];
	}
	else {
		return 1.0;
	}
}

SpaceFloat Agent::getAccelMultiplier() const
{
	if (crntState == agent_state::sprinting) {
		return (*this)[Attribute::sprintSpeedRatio] * (*this)[Attribute::sprintSpeedRatio];
	}
	else {
		return 1.0;
	}
}

bool Agent::canPlaceBomb(SpaceVect pos)
{
	if (!crntBomb) {
		return false;
	}

	return !isBulletObstacle(pos, crntBomb->dimensions.x);
}

void Agent::setShieldActive(bool v)
{
    if(v)
        block();
    else
        endBlock();
}

bool Agent::isShield(Bullet * b)
{
	if (get(Attribute::shieldLevel) <= 0.0f || !isShieldActive())
		return false;

	SpaceVect d = -1.0 * b->getVel().normalizeSafe();
	float cost = getShieldCost(d);

	if (cost == -1.0f || cost > get(Attribute::stamina))
		return false;

	modifyAttribute(Attribute::stamina, -cost);
	return true;
}

void Agent::initializeGraphics()
{
	animation = make_unique<AgentAnimationContext>(space);
	spriteID = animation->initializeGraphics(
		getSprite(),
		getRadius(),
		sceneLayer(),
		getInitialCenterPix()
	);
	animation->setAngle(prevAngle);
}

bool Agent::fire()
{
    FirePattern* fp = getFirePattern();
    bool fired = false;
    
    if (!fp) {
        log1("%s: Attempt to fire without FirePattern!", toString());
        return false;
    }

    float fireCost = getFirePattern()->getCost();
	bool inhibit = isActive(Attribute::inhibitFiring);
	bool hasEnergy = (*this)[Attribute::stamina] >= fireCost;
	bool validState = crntState == agent_state::none;

    if(inhibit || !hasEnergy || !validState){
        return false;
    }
    
    fired = fp->fireIfPossible();
    if (fired) {
        playSoundSpatial("sfx/shot.wav");
        consume(Attribute::stamina, fireCost);
    }

    return fired;
}

bool Agent::aimAtTarget(gobject_ref target)
{
    if (!target.isValid())
        return false;

    setAngle(ai::directionToTarget(this, target.get()->getPos()).toAngle());
    return true;
}

bool Agent::canSprint()
{
    return
        crntState == agent_state::none &&
		!isActive(Attribute::sprintCooldown) &&
		!space->getSuppressAction() &&
		(*this)[Attribute::stamina] >= (*this)[Attribute::sprintCost]
    ;
}

void Agent::sprint(SpaceVect direction)
{
    if(!canSprint())
        return;

    setState(agent_state::sprinting);
    stateData = sprint_data{direction};
    
    consume(Attribute::stamina, (*this)[Attribute::sprintCost]);
}

bool Agent::canBlock()
{
    return crntState == agent_state::none;
}

void Agent::block()
{
    if(crntState == agent_state::none && (*this)[Attribute::shieldLevel] > 0.0f){
        setState(agent_state::blocking);
    }
    
    space->addGraphicsAction(&graphics_context::setSpriteVisible, agentOverlay, true);
}

void Agent::endBlock()
{
    if(isShieldActive()){
        setState(agent_state::none);
    }
    
    space->addGraphicsAction(&graphics_context::setSpriteVisible, agentOverlay, false);
}

bool Agent::hasPowerAttack()
{
    return powerAttack != nullptr;
}

bool Agent::powerAttackAvailable()
{
    return hasPowerAttack() && canApplySpellCost(powerAttack->getCost());
}

bool Agent::doPowerAttack(const SpellDesc* p)
{
    if(crntState != agent_state::none)
        return false;
        
    power_attack_data data;
    data.attack = cast(p);
    
    if(data.attack){
        setState(agent_state::powerAttack);
        stateData = data;
    }
    
    return data.attack;
}

bool Agent::doPowerAttack()
{
    return doPowerAttack(powerAttack);
}

bool Agent::isBombAvailable()
{
    return
        crntBomb &&
        (crntState == agent_state::none || crntState == agent_state::blocking) &&
        !isActive(Attribute::bombCooldown) &&
        (*this)[Attribute::mp] >= crntBomb->cost &&
        !isActive(Attribute::inhibitFiring)
    ;
}

//0.0 means place it (if standing still), 1.0 means throw it at max throw speed
//either way, the agents current velocity will be added to it.
bool Agent::throwBomb(local_shared_ptr<bomb_properties> bomb, SpaceFloat speedRatio)
{
    SpaceVect bombPos = getPos() + SpaceVect::ray(bombSpawnDistance, getAngle());

    if(!isBombAvailable() || !canPlaceBomb(bombPos)){
        return false;
    }
    
    SpaceVect bombVel = getVel();
    bombVel += SpaceVect::ray((*this)[Attribute::maxThrowSpeed]*speedRatio, getAngle());

    space->createObject<Bomb>(
        object_params(bombPos,bombVel),
        bomb
    );
    consume(Attribute::mp, crntBomb->cost);
    
    attributeSystem->set(Attribute::bombCooldown, Attribute::throwInterval);

    return true;
}

void Agent::applyDesiredMovement(SpaceVect direction)
{
	if(
		crntState == agent_state::sprintRecovery ||
		crntState == agent_state::knockback ||
		crntState == agent_state::knockbackRecovery
	){
		return;
	}

    SpaceFloat speed = getMaxSpeed() * getSpeedMultiplier();
    SpaceFloat accel = getMaxAcceleration() * getAccelMultiplier();

    ai::applyDesiredVelocity(this, direction*speed, accel);
}

void Agent::applyStoppingForce()
{
    SpaceFloat accel = getMaxAcceleration() * getAccelMultiplier();

    ai::applyDesiredVelocity(this, SpaceVect::zero, accel);
}

bool Agent::canCast(const SpellDesc* spell)
{
    return
        !isActive(Attribute::inhibitSpellcasting) &&
        spell &&
        canApplySpellCost(spell->getCost()) &&
        !isActive(Attribute::spellCooldown)
    ;
}

void Agent::selectNextSpell()
{
    if(inventory->spells.hasItems()){
        equippedSpell = inventory->spells.next();
        
        if(dynamic_cast<Player*>(this))
            space->addHudAction(&HUD::setSpellIcon, inventory->spells.getIcon());
    }
}

void Agent::selectPrevSpell()
{
    if(inventory->spells.hasItems()){
        equippedSpell = inventory->spells.prev();
        
        if(dynamic_cast<Player*>(this))
            space->addHudAction(&HUD::setSpellIcon, inventory->spells.getIcon());
    }
}

void Agent::selectNextFirePattern()
{
    if(inventory->firePatterns.hasItems()){
        setFirePattern(inventory->firePatterns.prev());
        
        if(dynamic_cast<Player*>(this))
            space->addHudAction(&HUD::setFirePatternIcon, inventory->firePatterns.getIcon());
    }
}

void Agent::selectPrevFirePattern()
{
    if(inventory->firePatterns.hasItems()){
        setFirePattern(inventory->firePatterns.next());
        
        if(dynamic_cast<Player*>(this))
            space->addHudAction(&HUD::setFirePatternIcon, inventory->firePatterns.getIcon());
    }
}

void Agent::setState(agent_state newState)
{
	if(crntState != agent_state::none && crntState != newState)
		endState();

    crntState = newState;
    timeInState = 0.0;
}

void Agent::updateState()
{
    timerIncrement(timeInState);
    
    switch(crntState)
    {
    case agent_state::sprinting:
		_updateSprinting();
    break;
    case agent_state::sprintRecovery:
		_updateSprintRecovery();
    break;
    case agent_state::powerAttack:
		_updatePowerAttack();
    break;
    case agent_state::knockback:
		_updateKnockback();
    break;
    case agent_state::knockbackRecovery:
		_updateKnockbackRecovery();
	break;
    }
}

void Agent::endState()
{
	switch(crntState)
	{
	case agent_state::powerAttack:
		_endPowerAttack();
	break;
	}
}

bool Agent::isShieldActive()
{
    return crntState == agent_state::blocking;
}

bool Agent::isSprintActive()
{
    return crntState == agent_state::sprinting;
}

void Agent::setAngle(SpaceFloat a)
{
	GObject::setAngle(a);

	if (animation) {
		animation->setDirection(angleToDirection(a));
	}
}

void Agent::setDirection(Direction d)
{
	GObject::setDirection(d);

	if (animation) {
		animation->setDirection(d);
	}
}

void Agent::resetAnimation()
{
	if (animation) {
		animation->reset();
	}
}

void Agent::setSprite(shared_ptr<sprite_properties> sprite)
{
	if (animation) {
		animation->setSprite(sprite);
	}
}

//shield
//1 - deflect 45deg for 10
//2 - deflect 90deg for 10 & 45deg for 5
//3 - deflect 135deg for 12.5, 90 for 7.5 & 45 deg for 2.5
//4 - deflect 180deg for 15, 135 for 10, 90 for 5 & 45 for 0

const vector<vector<float>> shieldCosts = {
	{10.0f, -1.0f, -1.0f, -1.0f},
	{5.0f, 10.0f, -1.0f, -1.0f},
	{2.5f, 7.5f, 12.5f, -1.0f},
	{0.0f, 5.0f, 10.0f, 15.0f},
};

const array<SpaceFloat, 4> shieldScalars = {
	boost::math::double_constants::one_div_root_two,
	0.0, 
	-boost::math::double_constants::one_div_root_two,
	-1.0
};

float Agent::getShieldCost(SpaceVect n)
{
	SpaceFloat scalar = SpaceVect::dot(n, SpaceVect::ray(1.0, getAngle()));
	int level = min<int>( (*this)[Attribute::shieldLevel], shieldCosts.size()+1);

	for_irange(i,0,shieldScalars.size()) {
		if (scalar >= shieldScalars[i]) {
			return shieldCosts[level - 1][i];
		}
	}

	//Shouldn't happen, since scalar product should always be >= -1.0
	return shieldCosts[3][level - 1];
}

void Agent::onBulletCollide(Bullet* b, SpaceVect n)
{
	DamageInfo damage = b->getScaledDamageInfo();

	if (!isShield(b)) {
		modifyAttribute(Attribute::stress, Attribute::stressFromHits, damage.mag);
		hit(damage, n);
		runMethodIfAvailable("onBulletHit", b);
	}
	else {
		modifyAttribute(Attribute::stress, Attribute::stressFromBlocks, damage.mag);
  		runMethodIfAvailable("onBulletBlock", b);
	}
}

void Agent::onTouchAgent(Agent* other)
{
	other->hit(touchEffect(), ai::directionToTarget(this, other->getPos()));
}

void Agent::onEndTouchAgent(Agent* other)
{
}

bool Agent::isEnemy(Agent* other)
{
	return (
		dynamic_cast<Enemy*>(this) && dynamic_cast<Player*>(other) || 
		dynamic_cast<Player*>(this) && dynamic_cast<Enemy*>(other)
	);
}

bool Agent::isEnemyBullet(Bullet* other)
{
	return (
		type == GType::player && other->getType() == GType::enemyBullet ||
		type == GType::enemy && other->getType() == GType::playerBullet
	);
}

void Agent::applyKnockback(SpaceVect f)
{
	if(f.isZero() || getMass() <= 0.0) return;
	
	applyImpulse(f);
	float knockbackTime = f.length() / getMass() * (*this)[Attribute::knockbackSensitivity];
	
	if(crntState != agent_state::knockback && knockbackTime >= minKnockbackTime){
		setState(agent_state::knockback);
		stateData = knockback_data{};
	}
	if(crntState == agent_state::knockback) {
		float& accumulator = std::get<knockback_data>(stateData).accumulator;
		accumulator += knockbackTime;
	}
}

bool Agent::hit(DamageInfo damage, SpaceVect n)
{
	applyKnockback(n * damage.knockback);

	if (attributeSystem->isNonzero(Attribute::hitProtection) || damage.mag == 0.0f)
		return false;

	attributeSystem->set(Attribute::combo, 0.0f);

	if (dynamic_cast<Enemy*>(this)) {
		damage.mag /= app::params.difficultyScale;
	}
	else if (dynamic_cast<Player*>(this)) {
		damage.mag *= app::params.difficultyScale;
	}

	float hp = attributeSystem->applyDamage(damage);

	if (!damage.damageOverTime && hp > 0.0f) {
		space->addGraphicsAction(&graphics_context::createDamageIndicator, hp, getPos());
	}

	return true;
}

void Agent::applyAttributeEffects(AttributeMap attributeEffect)
{
	attributeSystem->apply(attributeEffect);
}

DamageInfo Agent::touchEffect() const
{
	return DamageInfo();
}

SpaceFloat Agent::getSensedObjectDistance(GType type) const {
	return radar->getSensedObjectDistance(type);
}

void Agent::initializeRadar()
{
	auto attr = sensor_attributes{
		getRadarRadius(),
		getDefaultFovAngle(),
		hasEssenceRadar()
	};

	if (attr.radius <= 0.0) {
		log1("%s has zero radius", getName());
		return;
	}

	radar = allocator_new<RadarSensor>(
		this,
		attr,
		bind(&Agent::onDetect, this, placeholders::_1),
		bind(&Agent::onEndDetect, this, placeholders::_1)
	);

	if (radar) {
		space->insertSensor(radar);
	}
}

void Agent::removePhysicsObjects()
{
	GObject::removePhysicsObjects();

	if (radar) {
		space->removeSensor(radar);
		allocator_delete(radar);
	}
}

void Agent::updateAnimation()
{
	if (!animation) return;

	SpaceVect dist = getVel()*app::params.secondsPerFrame;
	bool advance = animation->accumulate(dist.length());

	if (advance && isOnFloor && crntFloorCenterContact) {
		string sfxRes = crntFloorCenterContact->getFootstepSfx();
		if (!sfxRes.empty()) {
			playSoundSpatial(sfxRes, 0.5f, false, -1.0f);
		}
	}
}

void Agent::_updateSprinting()
{
	applyDesiredMovement(std::get<sprint_data>(stateData).sprintDirection);

	if(timeInState > (*this)[Attribute::sprintTime]) {
		setState(agent_state::sprintRecovery);
	}
}

void Agent::_updateSprintRecovery()
{
	applyStoppingForce();

	if(timeInState > (*this)[Attribute::sprintRecoveryTime]){
		setState(agent_state::none);
		setAttribute(Attribute::sprintCooldown, Attribute::sprintCooldownTime);
	}
}

void Agent::_updatePowerAttack()
{
	auto attack = std::get<power_attack_data>(stateData).attack;
	if(!attack->isSpellActive()) {
		setState(agent_state::none);
	}
	else{
		attack->runUpdate();
	}
}

void Agent::_updateKnockback()
{
	applyStoppingForce();

	if(getVel() == SpaceVect::zero)
		setState(agent_state::knockbackRecovery);
}

void Agent::_updateKnockbackRecovery()
{
	applyStoppingForce();

	float& accumulator = std::get<knockback_data>(stateData).accumulator;
	timerDecrement(accumulator);
	
	if(accumulator <= 0.0f || timeInState >= maxKnockbackTime){
		setState(agent_state::none);
	}
}

void Agent::_endPowerAttack()
{
	auto attack = std::get<power_attack_data>(stateData).attack;
	
	if(attack->isSpellActive()){
		attack->stop();
	}
}
