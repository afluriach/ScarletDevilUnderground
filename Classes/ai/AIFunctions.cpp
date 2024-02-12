//
//  AIFunctions.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "app_constants.hpp"
#include "AreaSensor.hpp"
#include "audio_context.hpp"
#include "Bomb.hpp"
#include "Bullet.hpp"
#include "FirePattern.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "HUD.hpp"
#include "Inventory.hpp"
#include "LuaAPI.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "RadarSensor.hpp"
#include "SpellDescriptor.hpp"
#include "SpellUtil.hpp"
#include "sol_util.hpp"
#include "value_map.hpp"

namespace ai{

ScriptFunction::ScriptFunction(GObject* object, const string& cls) :
	Function(object),
	cls(cls)
{
	Function* f_this = this;

	obj = GSpace::scriptVM->_state["ai"][cls](f_this);
}

ScriptFunction::ScriptFunction(GObject* object, GObject* target, const string& cls) :
	Function(object),
	cls(cls)
{
	Function* f_this = this;

	obj = GSpace::scriptVM->_state["ai"][cls](f_this, gobject_ref(target));
}

void ScriptFunction::onEnter()
{
    sol::runMethodIfAvailable(obj,"onEnter");
}

void ScriptFunction::update()
{
    sol::runMethodIfAvailable(obj, "update");
}

void ScriptFunction::onExit()
{
    sol::runMethodIfAvailable(obj, "onExit");
}

string ScriptFunction::getName()
{
    return sol::runMethodIfAvailableOrDefault(cls, obj, "getName");
}

bool ScriptFunction::hasMethod(const string& name)
{
	sol::object f = obj[name.c_str()];
	bool result = f.valid();
	return result;
}

AgentFunction::AgentFunction(GObject* object) :
    Function(object),
    agent(object->getAs<Agent>())
{
    if(!agent){
        logAndThrowError("Object is not an Agent!");
    }
}

PlayerFunction::PlayerFunction(GObject* object) :
    Function(object),
    player(object->getAs<Player>())
{
    if(!player){
        logAndThrowError("Object is not Player!");
    }
}

Seek::Seek(GObject* object, GObject* target, bool usePathfinding, SpaceFloat margin) :
	Function(object),
	target(target),
	usePathfinding(usePathfinding),
	margin(margin)
{}

void Seek::update()
{
	timerDecrement(lastPathfind);

	if (!target.isValid()) {
		crntState = states::no_target;
	}
	else if (crntState == states::pathfinding && !isObstacleBetweenTarget(object, target.get())) {
		crntState = states::direct_seek;
		pathFunction.reset();
	}
	else if (
		usePathfinding &&
		crntState == states::direct_seek &&
		lastPathfind <= 0.0 &&
		isObstacleBetweenTarget(object, target.get())
	){
		pathFunction = ai::FollowPath::pathToTarget(object, target.get());
		lastPathfind = pathfindingCooldown;

		if (pathFunction) {
			crntState = states::pathfinding;
		}
	}
	else if (distanceToTarget(object, target.get()->getPos()) < margin) {
		crntState = states::arriving;
	}
	else if (crntState == states::arriving && distanceToTarget(object, target.get()->getPos()) > margin) {
		crntState = states::direct_seek;
	}

	switch (crntState)
	{
	case states::direct_seek:
		seek(
			object,
			target.get()->getPos(),
			object->getMaxSpeed(),
			object->getMaxAcceleration()
		);
	break;
	case states::pathfinding:
		pathFunction->update();
	break;
	case states::arriving:
		arrive(object, target.get()->getPos());
	break;
	}

	if (crntState != states::no_target) {
		object->setDirection(toDirection(
            ai::directionToTarget(
                object,
                target.get()->getPos()
            )
        ));
	}

	if (!target.isValid())
        _state = state::completed;
}

MaintainDistance::MaintainDistance(GObject* object, gobject_ref target, SpaceFloat distance, SpaceFloat margin) :
Function(object),
target(target),
distance(distance),
margin(margin)
{}

void MaintainDistance::update()
{
	if (target.get()) {
        SpaceFloat crnt_distance = distanceToTarget(object,target.get());
		SpaceFloat stop_dist = getStoppingDistance(object);
    
		if (abs(crnt_distance - distance) < stop_dist) {
			ai::arrive(
				object,
				SpaceVect::ray(distance, float_pi + directionToTarget(object, target.get()->getPos()).toAngle())
			);
		}

        else if(crnt_distance > distance + margin){
            ai::seek(
                object,
                target.get()->getPos(),
                object->getMaxSpeed(),
                object->getMaxAcceleration()
            );
        }
        else if(crnt_distance < distance + margin){
            ai::fleeWithObstacleAvoidance(
                object,
                target.get()->getPos(),
                object->getMaxSpeed(),
                object->getMaxAcceleration()
            );
        }
	}
	else {
		ai::applyDesiredVelocity(object, SpaceVect::zero, object->getMaxAcceleration());
	}
}

OccupyPoint::OccupyPoint(GObject* object, SpaceVect target) :
	Function(object),
	target(target)
{
}

void OccupyPoint::update()
{
	GObject* object = object;
	SpaceFloat crnt_distance = distanceToTarget(object, target);
	SpaceFloat stop_dist = getStoppingDistance(object);

	if (crnt_distance > stop_dist) {
		seek(object, target, object->getMaxSpeed(), object->getMaxAcceleration());
	}
	else {
		arrive(object, target);
	}
}

OccupyMidpoint::OccupyMidpoint(GObject* object, gobject_ref target1, gobject_ref target2) :
Function(object),
target1(target1),
target2(target2)
{
}

void OccupyMidpoint::update()
{
	GObject* t1 = target1.get();
	GObject* t2 = target2.get();

	if (!t1 || !t2) {
        _state = state::completed;
        return;
	}

	SpaceVect midpoint = (t1->getPos() + t2->getPos()) / 2.0;
	SpaceFloat crnt_distance = distanceToTarget(object, midpoint);
	SpaceFloat stop_dist = getStoppingDistance(object);

	if (crnt_distance > stop_dist) {
		seek(object, midpoint, object->getMaxSpeed(), object->getMaxAcceleration());
	}
	else {
		arrive(object, midpoint);
	}
}

Scurry::Scurry(GObject* object, GObject* _target, SpaceFloat _distance, SpaceFloat length) :
Function(object),
distance(_distance),
target(_target)
{
	startFrame = getSpace()->getFrame();

	if (length > 0.0)
		endFrame = startFrame + app::params.framesPerSecond*length;
	else
		endFrame = 0;
}

void Scurry::update()
{
	autoUpdateFunction(moveFunction);

	if (!target.isValid() || endFrame != 0 && getSpace()->getFrame() >= endFrame) {
        _state = state::completed;
        return;
	}

	if (moveFunction)
		return;

	SpaceVect displacement = displacementToTarget(object, target.get()->getPos());

	SpaceFloat angle = displacement.toAngle();
	if (!scurryLeft) {
		angle += float_pi;
	}
	scurryLeft = !scurryLeft;

	array<SpaceFloat, 8> obstacleFeelers = obstacleFeeler8(object, distance);
	int direction = chooseBestDirection(obstacleFeelers, angle, distance);

	if (direction != -1) {
		moveFunction = object->make<MoveToPoint>(
			object->getPos() + SpaceVect::ray(distance, direction * float_pi / 4.0)
		);
	}
}

Flee::Flee(GObject* object, GObject* target, SpaceFloat distance) :
	Function(object),
	target(target),
	distance(distance)
{}

void Flee::update()
{
	if (target.isValid()) {
		ai::fleeWithObstacleAvoidance(
			object,
			target.get()->getPos(),
			object->getMaxSpeed(),
			object->getMaxAcceleration()
		);
		object->setDirection(toDirection(
            ai::directionToTarget(
                object,
                target.get()->getPos()
            )
        ));
	}
	
	if (!target.isValid())
        _state = state::completed;
}

Evade::Evade(GObject* object, GType type) :
	AgentFunction(object),
	type(type)
{}

void Evade::update()
{
	const object_list* objs = agent->getRadar()->getSensedObjectsByGtype(type);
	
	if (!objs || objs->size() == 0) {
        _state = state::completed;
        return;
	}

	GObject* closest = nullptr;
	SpaceFloat closestDistance = numeric_limits<SpaceFloat>::infinity();
	 
	for(GObject* obj: *objs)
	{
		SpaceFloat crntDist = distanceToTarget(obj, object);

		if (crntDist < closestDistance) {
			closestDistance = crntDist;
			closest = obj;
		}
	}

	active = closest != nullptr;

	if (closest != nullptr)
	{
		SpaceVect offset = projectileEvasion(closest, object);
		if (!offset.isZero()) {
			applyDesiredVelocity(
				object,
				offset.normalize()*-1.0f * object->getMaxSpeed(),
				object->getMaxAcceleration()
			);
		}
	}
}

LookAround::LookAround(GObject* object, SpaceFloat angularVelocity) :
Function(object),
angularVelocity(angularVelocity)
{
}

void LookAround::update()
{
	object->rotate(angularVelocity * app::params.secondsPerFrame);
}

Flank::Flank(
	GObject* object,
	gobject_ref target,
	SpaceFloat desiredDistance,
	SpaceFloat wallMargin
) :
	Function(object),
	target(target),
	desiredDistance(desiredDistance),
	wallMargin(wallMargin)
{
}

void Flank::init()
{
}

void Flank::update()
{
	if (!target.isValid()) {
        _state = state::completed;
        return;
	}

	autoUpdateFunction(moveFunction);

	if (moveFunction)
		return;

	SpaceVect target_pos;
	SpaceVect pos = target.get()->getPos();
	SpaceFloat angle = target.get()->getAngle();
	SpaceFloat this_angle = viewAngleToTarget(target.get(), object);

	SpaceVect rear_pos = SpaceVect::ray(desiredDistance, angle + float_pi) + pos;
	SpaceVect left_pos = SpaceVect::ray(desiredDistance, angle - float_pi / 2.0) + pos;
	SpaceVect right_pos = SpaceVect::ray(desiredDistance, angle + float_pi / 2.0) + pos;

	if (abs(this_angle) < float_pi / 4.0) {
		//move to side flank

		if (this_angle < 0 && !wallQuery(left_pos)) {
			target_pos = left_pos;
		}
		else if(!wallQuery(right_pos)) {
			target_pos = right_pos;
		}
	}
	else if(!wallQuery(rear_pos))
	{
		//move to rear flank
		target_pos = rear_pos;
	}
	if (!target_pos.isZero()) {
		moveFunction = object->make<MoveToPoint>(target_pos);
	}
	else {
		applyDesiredVelocity(object, SpaceVect::zero, object->getMaxAcceleration());
	}
}


bool Flank::wallQuery(SpaceVect pos)
{
	return getPhys()->obstacleRadiusQuery(
		object,
		pos, 
		wallMargin,
		GType::wall,
		PhysicsLayers::all
	);
}

LookTowardsFire::LookTowardsFire(GObject* object, bool useShield) :
	AgentFunction(object),
	useShield(useShield)
{
}

void LookTowardsFire::onEnter()
{
}

void LookTowardsFire::update()
{
	hitAccumulator -= (looking*lookTimeCoeff + (1-looking)*timeCoeff)* app::params.secondsPerFrame;
	hitAccumulator = max(hitAccumulator, 0.0f);

	if (hitAccumulator == 0.0f) {
		directionAccumulator = SpaceVect::zero;
		looking = false;
		if (useShield) agent->setShieldActive(false);
	}
	else if (!looking && hitAccumulator >= 1.0f) {
		object->setAngle(directionAccumulator.toAngle());
		looking = true;
		if (useShield) agent->setShieldActive(true);
	}

	if (looking) {
		applyDesiredVelocity(object, SpaceVect::zero, object->getMaxAcceleration());
	}
}

void LookTowardsFire::onExit()
{
}

bool LookTowardsFire::bulletHit(Bullet* b)
{
	SpaceVect bulletDirection = b->getVel().normalizeSafe().rotate(float_pi);
	hitAccumulator += hitCost;
	directionAccumulator += bulletDirection;

	if (looking) {
		object->setAngle(bulletDirection.toAngle());
	}

	return true;
}

const double MoveToPoint::arrivalMargin = 0.125;

MoveToPoint::MoveToPoint(GObject* object, SpaceVect target) :
	Function(object),
	target(target)
{}

void MoveToPoint::update()
{
    bool arrived = moveToPoint(object, target, arrivalMargin, false);
	
	if (arrived)
        _state = state::completed;
}

local_shared_ptr<FollowPath> FollowPath::pathToTarget(
	GObject* object,
	gobject_ref target
){
	if (!target.isValid()) {
		return nullptr;
	}

    return pathToPoint(object, target.get()->getPos());
}

local_shared_ptr<FollowPath> FollowPath::pathToPoint(
    GObject* object,
    SpaceVect point
){
	auto path = object->getSpace()->pathToTile(
		toIntVector(object->getPos()),
		toIntVector(point)
	);

	if (!path) {
		log2("%s (%u) no path to target", object->getName(), object->getUUID());
		return nullptr;
	}

	return make_local_shared<FollowPath>(
		object,
		path,
		false,
		false
	);
}

FollowPath::FollowPath(GObject* object, shared_ptr<const Path> path, bool loop, bool stopForObstacle) :
	Function(object),
	path(path),
	loop(loop),
	stopForObstacle(stopForObstacle)
{}

void FollowPath::update()
{
	if (currentTarget < path->size()) {
		object->setDirection(toDirection(ai::directionToTarget(object, (*path)[currentTarget])));
		bool arrived = moveToPoint(object, (*path)[currentTarget], MoveToPoint::arrivalMargin, stopForObstacle);
		currentTarget += arrived;
	}
	else if (loop && path->size() > 0) {
		currentTarget = 0;
	}
	else {
        _state = state::completed;
	}
}

Wander::Wander(GObject* object, SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist) :
	Function(object),
	minWait(minWait),
	maxWait(maxWait),
	minDist(minDist),
	maxDist(maxDist)
{
}

Wander::Wander(GObject* object, SpaceFloat waitInterval, SpaceFloat moveDist) :
	Wander(object, waitInterval, waitInterval, moveDist, moveDist)
{}

Wander::Wander(GObject* object) :
	Wander(object, 1.0, 1.0, 1.0, 1.0)
{}

pair<Direction, SpaceFloat> Wander::chooseMovement()
{
	array<SpaceFloat, 4> feelers = ai::obstacleFeelerQuad(object, maxDist);
	vector<Direction> directions;
	directions.reserve(4);

	//First, select a direction that allows maximum movement, if possible.
	enum_foreach(Direction, d, right, end)
	{
		if (feelers[to_int(d) - 1] == maxDist) {
			directions.push_back(d);
		}
	}

	if (!directions.empty()) {
		return make_pair(directions.at(getSpace()->getRandomInt(0, directions.size()-1)), maxDist);
	}

	//Select a direction that allows at least minimum desired distance.
	enum_foreach(Direction, d, right, end)
	{
		if (feelers[to_int(d) - 1] >= minDist) {
			directions.push_back(d);
		}
	}

	if (!directions.empty()) {
		int idx = getSpace()->getRandomInt(0, directions.size() - 1);
		return make_pair(directions.at(idx), feelers[idx]);
	}

	return make_pair(Direction::none, 0.0);
}

void Wander::update()
{
	autoUpdateFunction(moveFunction);

	if (!moveFunction) {
		timerDecrement(waitTimer);
		applyDesiredVelocity(object, SpaceVect::zero, object->getMaxAcceleration());
	}

	if (waitTimer <= 0.0 && !moveFunction) {
		pair<Direction, SpaceFloat> movement = chooseMovement();

		if (movement.first != Direction::none && movement.second > 0.0) {
			object->setDirection(movement.first);
			waitTimer = getSpace()->getRandomFloat(minWait, maxWait);
			moveFunction = object->make<MoveToPoint>(
				object->getPos() + dirToVector(movement.first)*movement.second
			);
		}
	}
}

void Wander::reset()
{
	moveFunction.reset();
}

Wait::Wait(GObject* object, SpaceFloat duration) :
    Function(object),
    duration(duration)
{
}

void Wait::update()
{
    timerIncrement(t);
    ai::applyDesiredVelocity(object, SpaceVect::zero, object->getMaxAcceleration());
    
    if(t >= duration)
        _state = state::completed;
}

FireAtTarget::FireAtTarget(GObject* object, gobject_ref target) :
	AgentFunction(object),
	target(target)
{}

void FireAtTarget::update()
{
	FirePattern* fp = agent->getFirePattern();
	if (!target.isValid() || !fp) {
		_state = state::completed;
        return;
	}

	object->setAngle(
		directionToTarget(object, target.get()->getPos()).toAngle()
	);

	agent->fire();
}

FireOnStress::FireOnStress(GObject* object, float stressPerShot) :
	AgentFunction(object),
	stressPerShot(stressPerShot)
{
}

void FireOnStress::update()
{
	if (agent->get(Attribute::stress) >= stressPerShot && agent->fire()) {
		agent->modifyAttribute(Attribute::stress, -stressPerShot);
	}
}

ThrowBombs::ThrowBombs(
	GObject* object,
	gobject_ref target,
	local_shared_ptr<bomb_properties> bombType,
	SpaceFloat throwingSpeed,
	SpaceFloat baseInterval
) :
	AgentFunction(object),
	target(target),
	bombType(bombType),
	throwingSpeed(throwingSpeed),
	baseInterval(baseInterval)
{
}

void ThrowBombs::init()
{
	countdown = getInterval();
}

void ThrowBombs::update()
{
	if (!target.isValid() || !bombType) {
        _state = state::completed;
        return;
	}

	SpaceFloat fuseTime = bombType->fuseTime;
	SpaceFloat blastRadius = bombType->blastRadius;

	timerDecrement(countdown);

	if (countdown <= 0.0 && agent->get(Attribute::mp) >= bombType->cost) {
		SpaceFloat angle = directionToTarget(object, target.get()->getPos()).toAngle();
		SpaceVect pos = object->getPos() + SpaceVect::ray(1.0, angle);
		SpaceVect vel = object->getVel() + SpaceVect::ray(throwingSpeed, angle);
		bool hasEnoughMagic =
			agent->getAttributeSystem()->getHealthRatio() <
			agent->getAttributeSystem()->getMagicRatio()
		;

		if (
			//can place bomb
			!getPhys()->obstacleRadiusQuery(
				object,
				pos,
				0.5,
				obstacles,
				PhysicsLayers::ground
			) &&
			//bomb is likely to travel a significant distance
			getPhys()->obstacleDistanceFeeler(
				object,
				SpaceVect::ray(1.0 + fuseTime*throwingSpeed, angle)
			) > blastRadius &&
			//predict net gain player-enemy damage
			score(object->getPos(), angle) > 0.5f &&
			//use bombs gradually in a fight
			hasEnoughMagic &&
			//do not throw if target is too close
			distanceToTarget(object, target.get()) > blastRadius
		) {
			SpaceVect bombPos = object->getPos() + SpaceVect::ray(1.0, angle);
			SpaceVect bombVel = object->getVel() + SpaceVect::ray(throwingSpeed, angle);

			getSpace()->createObject<Bomb>(
				object_params(bombPos, bombVel),
				bombType
			);

			countdown = getInterval();
		}
	}
}

SpaceFloat ThrowBombs::getInterval()
{
	return baseInterval / agent->get(Attribute::attackSpeed);
}

float ThrowBombs::score(SpaceVect pos, SpaceFloat angle)
{
	SpaceVect predictedPos = pos + SpaceVect::ray(1.0 + bombType->fuseTime * throwingSpeed, angle);
	return bombScore(getSpace(), predictedPos, bombType->blastRadius);
}

PlayerControl::PlayerControl(GObject* object) :
	PlayerFunction(object)
{
    if(!player){
        logAndThrowError("Agent is not a Player object.");
    }
}

PlayerControl::~PlayerControl()
{
}

void PlayerControl::onEnter()
{
}

void PlayerControl::update()
{
    ControlInfo cs = getSpace()->getControlInfo();
    
    if(activeSpell && !activeSpell->isSpellActive()){
        activeSpell.reset();
        player->setAttribute(Attribute::spellCooldown, Attribute::castInterval);
    }
    else if(activeSpell){
        activeSpell->runUpdate();
    }

    checkBlockControls(cs);
    checkMovementControls(cs);
    checkItemInteraction(cs);
    
    if (!getSpace()->getSuppressAction()) {
        checkFireControls(cs);
        checkBombControls(cs);
        updateSpellControls(cs);
    }
    
    applyDesiredMovement();
}

void PlayerControl::checkBlockControls(const ControlInfo& cs)
{
    if(
        cs.isControlActionDown(ControlAction::focus) &&
        !player->isShieldActive() &&
        player->canBlock()
    ){
        player->block();
    }
    else if(
        !cs.isControlActionDown(ControlAction::focus) &&
        player->isShieldActive()
    ){
        player->endBlock();
    }
    
    if (cs.isControlActionPressed(ControlAction::focus)) {
		getSpace()->setBulletBodiesVisible(true);
	}
	else if (cs.isControlActionReleased(ControlAction::focus)) {
		getSpace()->setBulletBodiesVisible(false);
	}
}

void PlayerControl::checkMovementControls(const ControlInfo& cs)
{
	if (player->isActive(Attribute::inhibitMovement)){
		return;
	}

    desiredMoveDirection = cs.left_v;
	SpaceVect facing = cs.isControlActionDown(ControlAction::center_look) ?
		cs.left_v : cs.right_v;
	
	if (player->canSprint() &&
		cs.isControlActionDown(ControlAction::sprint) &&
		!desiredMoveDirection.isZero()
	) {
        player->sprint(desiredMoveDirection);
	}
    
	if (desiredMoveDirection.isZero()) {
		player->resetAnimation();
	}
    	
	if (facing.lengthSq() > 0.0) {
		player->setAngle(facing.toAngle());
	}
}

void PlayerControl::checkFireControls(const ControlInfo& cs)
{
    Inventory* inventory = player->getInventory();
    GSpace* space = getSpace();

	bool isFireButton =
		cs.isControlActionDown(ControlAction::fire) ||
		isAutoFire && cs.right_v.lengthSq() >= ControlRegister::deadzone2
	;

	if (cs.isControlActionPressed(ControlAction::fire_mode)) {
		isAutoFire = !isAutoFire;
	}

	//Fire if arrow key is pressed
	if ( isFireButton && player->fire()
    ){
        ;
	}
	else if (cs.isControlActionPressed(ControlAction::fire_pattern_previous)){
        player->selectPrevFirePattern();
	}
	else if (cs.isControlActionPressed(ControlAction::fire_pattern_next)){
        player->selectNextFirePattern();
	}
	else if (
		!player->isActive(Attribute::inhibitFiring) &&
		cs.isControlActionPressed(ControlAction::power_attack) &&
		player->hasPowerAttack()
	){
        player->doPowerAttack();
    }
}

void PlayerControl::checkBombControls(const ControlInfo& cs)
{
	if (cs.isControlActionPressed(ControlAction::bomb)) {
        SpaceFloat speedRatio = cs.isControlActionDown(ControlAction::focus) ? 1.0 : 0.0;
        player->throwBomb(player->getBomb(), speedRatio);
	}
}

void PlayerControl::checkItemInteraction(const ControlInfo& cs)
{
    if(cs.isControlActionPressed(ControlAction::interact)){
        tryInteract();
    }
}

void PlayerControl::updateSpellControls(const ControlInfo& cs)
{
	if (cs.isControlActionPressed(ControlAction::spell)) {
        toggleSpell();
	}
	else if (cs.isControlActionPressed(ControlAction::spell_previous)) {
        player->selectPrevSpell();
    }
    else if (cs.isControlActionPressed(ControlAction::spell_next)) {
        player->selectNextSpell();
    }		
}

void PlayerControl::applyDesiredMovement()
{
    player->applyDesiredMovement(desiredMoveDirection);
}

bool PlayerControl::tryInteract()
{
	timerDecrement(interactCooldown);
    bool result = false;

	GObject* interactible = getSpace()->physicsContext->interactibleObjectFeeler(
		player,
		player->getInteractFeeler()
	);
	
	if(interactible && interactible->canInteract(player))
    {
        if(interactCooldown <= 0.0){
            interactible->interact(player);
            interactCooldown = Player::interactCooldownTime;
            result = true;
        }
    }

	getSpace()->addHudAction(
		&HUD::setInteractionIcon,
		interactible && interactible->canInteract(player) ? interactible->interactionIcon(player) : ""
	);
 
    return result;
}

void PlayerControl::toggleSpell()
{
    if(activeSpell){
        if(activeSpell->isSpellActive())
            activeSpell->stop();
        
        player->setAttribute(Attribute::spellCooldown, Attribute::castInterval);
        activeSpell.reset();
    }
    else{
        auto spell = player->getInventory()->spells.getCrnt();
        
        if(spell && player->canCast(spell)){
            activeSpell = player->cast(spell);
        }
    }
}

}//end NS
