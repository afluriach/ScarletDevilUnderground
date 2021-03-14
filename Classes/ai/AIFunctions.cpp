//
//  AIFunctions.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
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
#include "LuaAPI.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "RadarSensor.hpp"
#include "SpellDescriptor.hpp"
#include "SpellUtil.hpp"
#include "value_map.hpp"

namespace ai{

AITargetFunctionGenerator ScriptFunction::targetGenerator(const string& cls)
{
	return makeTargetFunctionGenerator<ScriptFunction>(cls);
}

ScriptFunction::ScriptFunction(StateMachine* fsm, const string& cls) :
	Function(fsm),
	cls(cls)
{
	Function* f_this = this;

	obj = GSpace::scriptVM->_state["ai"][cls](f_this);
}

ScriptFunction::ScriptFunction(StateMachine* fsm, GObject* target, const string& cls) :
	Function(fsm),
	cls(cls)
{
	Function* f_this = this;

	obj = GSpace::scriptVM->_state["ai"][cls](f_this, gobject_ref(target));
}

void ScriptFunction::onEnter()
{
	sol::function f = obj["onEnter"];
	if (f) f(obj);
}

update_return ScriptFunction::update()
{
	sol::function f = obj["update"];

	if (f) {
		update_return result = f(obj);
		return result;
	}
	else {
		return_steady(-1.0f);
	}
}

bool ScriptFunction::bulletBlock(Bullet* b)
{
	sol::function f = obj["bulletBlock"];
	if (f) return f(obj, b);
	else return false;
}

bool ScriptFunction::bulletHit(Bullet* b)
{
	sol::function f = obj["bulletHit"];
	if (f) return f(obj, b);
	else return false;
}

bool ScriptFunction::detectEnemy(Agent* enemy)
{
	sol::function f = obj["detectEnemy"];
	if (f) return f(obj, enemy);
	else return false;
}

bool ScriptFunction::endDetectEnemy(Agent* enemy)
{
	sol::function f = obj["endDetectEnemy"];
	if (f) return f(obj, enemy);
	else return false;
}

bool ScriptFunction::detectBomb(Bomb* bomb)
{
	sol::function f = obj["detectBomb"];
	if (f) return f(obj, bomb);
	else return false;
}

bool ScriptFunction::detectBullet(Bullet* bullet)
{
	sol::function f = obj["detectBullet"];
	if (f) return f(obj, bullet);
	else return false;
}

bool ScriptFunction::enemyRoomAlert(Agent* enemy)
{
	sol::function f = obj["enemyRoomAlert"];
	if (f) return f(obj, enemy);
	else return false;
}

bool ScriptFunction::zeroHP()
{
	sol::function f = obj["zeroHP"];
	if (f) return f(obj);
	else return false;
}

bool ScriptFunction::zeroStamina()
{
	sol::function f = obj["zeroStamina"];
	if (f) return f(obj);
	else return false;
}

void ScriptFunction::onExit()
{
	sol::function f = obj["onExit"];
	if (f) {
		f(obj);
	}
}

string ScriptFunction::getName()
{
	sol::function f = obj["getName"];
	if (f) {
		string result = f(obj);
		return result;
	}
	else {
		return cls;
	}
}

bool ScriptFunction::hasMethod(const string& name)
{
	sol::object f = obj[name.c_str()];
	bool result = f.valid();
	return result;
}

Seek::Seek(StateMachine* fsm, GObject* target, bool usePathfinding, SpaceFloat margin) :
	Function(fsm),
	target(target),
	usePathfinding(usePathfinding),
	margin(margin)
{}

update_return Seek::update()
{
	GObject* agent = getObject();
	timerDecrement(lastPathfind);

	if (!target.isValid()) {
		crntState = states::no_target;
	}
	else if (crntState == states::pathfinding && !isObstacleBetweenTarget(agent, target.get())) {
		crntState = states::direct_seek;
		pathFunction.reset();
	}
	else if (
		usePathfinding &&
		crntState == states::direct_seek &&
		lastPathfind <= 0.0 &&
		isObstacleBetweenTarget(agent, target.get())
	){
		pathFunction = ai::FollowPath::pathToTarget(fsm, target.get());
		lastPathfind = pathfindingCooldown;

		if (pathFunction) {
			crntState = states::pathfinding;
		}
	}
	else if (distanceToTarget(agent, target.get()->getPos()) < margin) {
		crntState = states::arriving;
	}
	else if (crntState == states::arriving && distanceToTarget(agent, target.get()->getPos()) > margin) {
		crntState = states::direct_seek;
	}

	switch (crntState)
	{
	case states::direct_seek:
		seek(
			agent,
			target.get()->getPos(),
			agent->getMaxSpeed(),
			agent->getMaxAcceleration()
		);
	break;
	case states::pathfinding:
		pathFunction->update();
	break;
	case states::arriving:
		arrive(agent, target.get()->getPos());
	break;
	}

	if (crntState != states::no_target) {
		agent->setDirection(toDirection(
            ai::directionToTarget(
                agent,
                target.get()->getPos()
            )
        ));
	}

	if (target.isValid())
		return_steady(0.0f);
	else
		return_pop();
}

MaintainDistance::MaintainDistance(StateMachine* fsm, gobject_ref target, SpaceFloat distance, SpaceFloat margin) :
Function(fsm),
target(target),
distance(distance),
margin(margin)
{}

update_return MaintainDistance::update()
{
	Agent* agent = fsm->getAgent();
	if (target.get()) {
        SpaceFloat crnt_distance = distanceToTarget(agent,target.get());
		SpaceFloat stop_dist = getStoppingDistance(agent);
    
		if (abs(crnt_distance - distance) < stop_dist) {
			ai::arrive(
				agent,
				SpaceVect::ray(distance, float_pi + directionToTarget(agent, target.get()->getPos()).toAngle())
			);
		}

        else if(crnt_distance > distance + margin){
            ai::seek(
                agent,
                target.get()->getPos(),
                agent->getMaxSpeed(),
                agent->getMaxAcceleration()
            );
        }
        else if(crnt_distance < distance + margin){
            ai::fleeWithObstacleAvoidance(
                agent,
                target.get()->getPos(),
                agent->getMaxSpeed(),
                agent->getMaxAcceleration()
            );
        }
	}
	else {
		ai::applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}

	return_steady(0.0f);
}

OccupyPoint::OccupyPoint(StateMachine* fsm, SpaceVect target) :
	Function(fsm),
	target(target)
{
}

update_return OccupyPoint::update()
{
	GObject* agent = getObject();
	SpaceFloat crnt_distance = distanceToTarget(agent, target);
	SpaceFloat stop_dist = getStoppingDistance(agent);

	if (crnt_distance > stop_dist) {
		seek(agent, target, agent->getMaxSpeed(), agent->getMaxAcceleration());
	}
	else {
		arrive(agent, target);
	}

	return_steady(0.0f);
}

OccupyMidpoint::OccupyMidpoint(StateMachine* fsm, gobject_ref target1, gobject_ref target2) :
Function(fsm),
target1(target1),
target2(target2)
{
}

update_return OccupyMidpoint::update()
{
	GObject* t1 = target1.get();
	GObject* t2 = target2.get();
	GObject* agent = getObject();

	if (!t1 || !t2) {
		return_pop();
	}

	SpaceVect midpoint = (t1->getPos() + t2->getPos()) / 2.0;
	SpaceFloat crnt_distance = distanceToTarget(agent, midpoint);
	SpaceFloat stop_dist = getStoppingDistance(agent);

	if (crnt_distance > stop_dist) {
		seek(agent, midpoint, agent->getMaxSpeed(), agent->getMaxAcceleration());
	}
	else {
		arrive(agent, midpoint);
	}

	return_steady(0.0f);
}

Scurry::Scurry(StateMachine* fsm, GObject* _target, SpaceFloat _distance, SpaceFloat length) :
Function(fsm),
distance(_distance),
target(_target)
{
	startFrame = getSpace()->getFrame();

	if (length > 0.0)
		endFrame = startFrame + app::params.framesPerSecond*length;
	else
		endFrame = 0;
}

update_return Scurry::update()
{
	GObject* agent = getObject();
	autoUpdateFunction(moveFunction);

	if (!target.isValid() || endFrame != 0 && getSpace()->getFrame() >= endFrame) {
		return_pop();
	}

	if (moveFunction)
		return_steady(0.0f);

	SpaceVect displacement = displacementToTarget(agent, target.get()->getPos());

	SpaceFloat angle = displacement.toAngle();
	if (!scurryLeft) {
		angle += float_pi;
	}
	scurryLeft = !scurryLeft;

	array<SpaceFloat, 8> obstacleFeelers = obstacleFeeler8(agent, distance);
	int direction = chooseBestDirection(obstacleFeelers, angle, distance);

	if (direction != -1) {
		moveFunction = fsm->make<MoveToPoint>(
			agent->getPos() + SpaceVect::ray(distance, direction * float_pi / 4.0)
		);
	}

	return_steady(0.0f);
}

Flee::Flee(StateMachine* fsm, GObject* target, SpaceFloat distance) :
	Function(fsm),
	target(target),
	distance(distance)
{}

update_return Flee::update()
{
	GObject* agent = getObject();

	if (target.isValid()) {
		ai::fleeWithObstacleAvoidance(
			agent,
			target.get()->getPos(),
			agent->getMaxSpeed(),
			agent->getMaxAcceleration()
		);
		agent->setDirection(toDirection(
            ai::directionToTarget(
                agent,
                target.get()->getPos()
            )
        ));
	}
	
	if (target.isValid())
		return_steady(0.0f);
	else
		return_pop();
}

Evade::Evade(StateMachine* fsm, GType type) : 
	Function(fsm),
	type(type)
{}

update_return Evade::update()
{
	Agent* agent = getAgent();
	const object_list* objs = agent->getRadar()->getSensedObjectsByGtype(type);
	
	if (!objs || objs->size() == 0) {
		active = false;
		return_pop();
	}

	GObject* closest = nullptr;
	SpaceFloat closestDistance = numeric_limits<SpaceFloat>::infinity();
	 
	for(GObject* obj: *objs)
	{
		SpaceFloat crntDist = distanceToTarget(obj, agent);

		if (crntDist < closestDistance) {
			closestDistance = crntDist;
			closest = obj;
		}
	}

	active = closest != nullptr;

	if (closest != nullptr)
	{
		SpaceVect offset = projectileEvasion(closest, agent);
		if (!offset.isZero()) {
			applyDesiredVelocity(
				agent,
				offset.normalize()*-1.0f * agent->getMaxSpeed(),
				agent->getMaxAcceleration()
			);
		}
	}

	return_steady(0.0f);
}

LookAround::LookAround(StateMachine* fsm, SpaceFloat angularVelocity) :
Function(fsm),
angularVelocity(angularVelocity)
{
}

update_return LookAround::update()
{
	getObject()->rotate(angularVelocity * app::params.secondsPerFrame);
	return_steady(0.0f);
}

Flank::Flank(
	StateMachine* fsm,
	gobject_ref target,
	SpaceFloat desiredDistance,
	SpaceFloat wallMargin
) :
	Function(fsm),
	target(target),
	desiredDistance(desiredDistance),
	wallMargin(wallMargin)
{
}

void Flank::init()
{
}

update_return Flank::update()
{
	if (!target.isValid()) {
		return_pop();
	}

	autoUpdateFunction(moveFunction);

	if (moveFunction)
		return_steady(0.0f);

	GObject* agent = getObject();
	SpaceVect target_pos;
	SpaceVect pos = target.get()->getPos();
	SpaceFloat angle = target.get()->getAngle();
	SpaceFloat this_angle = viewAngleToTarget(target.get(), agent);

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
		moveFunction = fsm->make<MoveToPoint>(target_pos);
	}
	else {
		applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}

	return_steady(0.0f);
}


bool Flank::wallQuery(SpaceVect pos)
{
	return getPhys()->obstacleRadiusQuery(
		getObject(),
		pos, 
		wallMargin,
		GType::wall,
		PhysicsLayers::all
	);
}

LookTowardsFire::LookTowardsFire(StateMachine* fsm, bool useShield) :
	Function(fsm),
	useShield(useShield)
{
}

void LookTowardsFire::onEnter()
{
}

update_return LookTowardsFire::update()
{
	Agent* agent = getAgent();

	hitAccumulator -= (looking*lookTimeCoeff + (1-looking)*timeCoeff)* app::params.secondsPerFrame;
	hitAccumulator = max(hitAccumulator, 0.0f);

	if (hitAccumulator == 0.0f) {
		directionAccumulator = SpaceVect::zero;
		looking = false;
		if (useShield) agent->setShieldActive(false);
	}
	else if (!looking && hitAccumulator >= 1.0f) {
		agent->setAngle(directionAccumulator.toAngle());
		looking = true;
		if (useShield) agent->setShieldActive(true);
	}

	if (looking) {
		applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}

	return_steady(0.0f);
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
		getObject()->setAngle(bulletDirection.toAngle());
	}

	return true;
}

const double MoveToPoint::arrivalMargin = 0.125;

MoveToPoint::MoveToPoint(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
    auto xIter = args.find("target_x");
    auto yIter = args.find("target_y");
    
    if(xIter == args.end()){
        log("MoveToPoint::MoveToPoint: target_x missing from ValueMap");
        return;
    }
    if(yIter == args.end()){
        log("MoveToPoint::MoveToPoint: target_y missing from ValueMap");
        return;
    }
    
    const Value &x = xIter->second;
    const Value &y = yIter->second;

//Cocos2D ValueMap does not correctly read data type.
//    if(!x.isNumber()){
//        log("MoveToPoint::MoveToPoint: target_x is not a number.");
//        return;
//    }
//    if(!y.isNumber()){
//        log("MoveToPoint::MoveToPoint: target_y is not a number.");
//        return;
//    }
    
    target  = SpaceVect(x.asFloat(), y.asFloat());
}

MoveToPoint::MoveToPoint(StateMachine* fsm, SpaceVect target) :
	Function(fsm),
	target(target)
{}

update_return MoveToPoint::update()
{
	if (!arrived) {
		arrived = moveToPoint(getObject(), target, arrivalMargin, false);
	}
	
	if (arrived)
		return_pop();
	else
		return_steady(0.0f);
}

local_shared_ptr<FollowPath> FollowPath::pathToTarget(
	StateMachine* fsm,
	gobject_ref target
){
	if (!target.isValid()) {
		return nullptr;
	}
	Agent* agent = fsm->getAgent();

	Path path = fsm->getSpace()->pathToTile(
		toIntVector(agent->getPos()),
		toIntVector(target.get()->getPos())
	);

	if (path.empty()) {
		log("%s (%u) no path to target", agent->getName(), agent->getUUID());
		return nullptr;
	}

	return make_local_shared<FollowPath>(
		fsm,
		path,
		false,
		false
	);
}

FollowPath::FollowPath(StateMachine* fsm, Path path, bool loop, bool stopForObstacle) :
	Function(fsm),
	path(path),
	loop(loop),
	stopForObstacle(stopForObstacle)
{}

FollowPath::FollowPath(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
	auto name_it = args.find("pathName");
	auto loop_it = args.find("loop");

	if (name_it == args.end()) {
		log("FollowPath: pathName not provided!");
	}

	Path const* p = getSpace()->getPath(name_it->second.asString());

	if (!p) {
		log("FollowPath: pathName %s not found!", name_it->second.asString().c_str());
	}
	else {
		path = *p;
	}

	if (loop_it != args.end() && boost::lexical_cast<bool>(loop_it->second.asString())) {
		loop = true;
	}
}

update_return FollowPath::update()
{
	GObject* agent = getObject();

	if (currentTarget < path.size()) {
		agent->setDirection(toDirection(ai::directionToTarget(agent, path[currentTarget])));
		bool arrived = moveToPoint(agent, path[currentTarget], MoveToPoint::arrivalMargin, stopForObstacle);
		currentTarget += arrived;
	}
	else if (loop && path.size() > 0) {
		currentTarget = 0;
	}
	else {
		completed = true;
		return_pop();
	}

	return_steady(0.0f);
}

Wander::Wander(StateMachine* fsm, const ValueMap& args) :
	Function(fsm),
    init_float_field(minWait,1.0f),
    init_float_field(maxWait,1.0f),
    init_float_field(minDist,1.0f),
    init_float_field(maxDist,1.0f)
{}

Wander::Wander(StateMachine* fsm, SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist) :
	Function(fsm),
	minWait(minWait),
	maxWait(maxWait),
	minDist(minDist),
	maxDist(maxDist)
{
}

Wander::Wander(StateMachine* fsm, SpaceFloat waitInterval, SpaceFloat moveDist) :
	Wander(fsm, waitInterval, waitInterval, moveDist, moveDist)
{}

Wander::Wander(StateMachine* fsm) :
	Wander(fsm, 1.0, 1.0, 1.0, 1.0)
{}

pair<Direction, SpaceFloat> Wander::chooseMovement()
{
	array<SpaceFloat, 4> feelers = ai::obstacleFeelerQuad(getObject(), maxDist);
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
		return make_pair(directions.at(fsm->getSpace()->getRandomInt(0, directions.size()-1)), maxDist);
	}

	//Select a direction that allows at least minimum desired distance.
	enum_foreach(Direction, d, right, end)
	{
		if (feelers[to_int(d) - 1] >= minDist) {
			directions.push_back(d);
		}
	}

	if (!directions.empty()) {
		int idx = fsm->getSpace()->getRandomInt(0, directions.size() - 1);
		return make_pair(directions.at(idx), feelers[idx]);
	}

	return make_pair(Direction::none, 0.0);
}

update_return Wander::update()
{
	GObject* agent = getObject();
	autoUpdateFunction(moveFunction);

	if (!moveFunction) {
		timerDecrement(waitTimer);
		applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}

	if (waitTimer <= 0.0 && !moveFunction) {
		pair<Direction, SpaceFloat> movement = chooseMovement();

		if (movement.first != Direction::none && movement.second > 0.0) {
			agent->setDirection(movement.first);
			waitTimer = fsm->getSpace()->getRandomFloat(minWait, maxWait);
			moveFunction = fsm->make<MoveToPoint>(
				agent->getPos() + dirToVector(movement.first)*movement.second
			);
		}
	}

	return_steady(0.0f);
}

void Wander::reset()
{
	moveFunction.reset();
}

FireAtTarget::FireAtTarget(StateMachine* fsm, gobject_ref target) :
	Function(fsm),
	target(target)
{}

update_return FireAtTarget::update()
{
	Agent* agent = getAgent();
	FirePattern* fp = agent->getFirePattern();
	if (!target.isValid() || !fp) {
		return_pop();
	}

	agent->setAngle(
		directionToTarget(agent, target.get()->getPos()).toAngle()
	);

	fire();

	return_steady(0.0f);
}

FireOnStress::FireOnStress(StateMachine* fsm, float stressPerShot) :
	Function(fsm),
	stressPerShot(stressPerShot)
{
}

update_return FireOnStress::update()
{
	Agent* agent = getAgent();
	if (agent->get(Attribute::stress) >= stressPerShot && fire()) {
		agent->modifyAttribute(Attribute::stress, -stressPerShot);
	}
	return_steady(0.0f);
}

ThrowBombs::ThrowBombs(
	StateMachine* fsm,
	gobject_ref target,
	local_shared_ptr<bomb_properties> bombType,
	SpaceFloat throwingSpeed,
	SpaceFloat baseInterval
) :
	Function(fsm),
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

update_return ThrowBombs::update()
{
	Agent* agent = getAgent();

	if (!target.isValid() || !bombType) {
		return_pop();
	}

	SpaceFloat fuseTime = bombType->fuseTime;
	SpaceFloat blastRadius = bombType->blastRadius;

	timerDecrement(countdown);

	if (countdown <= 0.0 && agent->get(Attribute::mp) >= bombType->cost) {
		SpaceFloat angle = directionToTarget(agent, target.get()->getPos()).toAngle();
		SpaceVect pos = agent->getPos() + SpaceVect::ray(1.0, angle);
		SpaceVect vel = agent->getVel() + SpaceVect::ray(throwingSpeed, angle);
		bool hasEnoughMagic =
			agent->getAttributeSystem()->getHealthRatio() <
			agent->getAttributeSystem()->getMagicRatio()
		;

		if (
			//can place bomb
			!getPhys()->obstacleRadiusQuery(
				agent,
				pos,
				0.5,
				obstacles,
				PhysicsLayers::ground
			) &&
			//bomb is likely to travel a significant distance
			getPhys()->obstacleDistanceFeeler(
				agent,
				SpaceVect::ray(1.0 + fuseTime*throwingSpeed, angle)
			) > blastRadius &&
			//predict net gain player-enemy damage
			score(agent->getPos(), angle) > 0.5f &&
			//use bombs gradually in a fight
			hasEnoughMagic &&
			//do not throw if target is too close
			distanceToTarget(agent, target.get()) > blastRadius
		) {
			SpaceVect bombPos = agent->getPos() + SpaceVect::ray(1.0, angle);
			SpaceVect bombVel = agent->getVel() + SpaceVect::ray(throwingSpeed, angle);

			getSpace()->createObject<Bomb>(
				object_params(bombPos, bombVel),
				bombType
			);

			countdown = getInterval();
		}
	}
	return_steady(0.0f);
}

SpaceFloat ThrowBombs::getInterval()
{
	return baseInterval / (*getAgent()->getAttributeSystem())[Attribute::attackSpeed];
}

float ThrowBombs::score(SpaceVect pos, SpaceFloat angle)
{
	SpaceVect predictedPos = pos + SpaceVect::ray(1.0 + bombType->fuseTime * throwingSpeed, angle);
	return bombScore(getSpace(), predictedPos, bombType->blastRadius);
}

}//end NS
