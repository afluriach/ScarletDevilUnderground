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
#include "GSpace.hpp"
#include "physics_context.hpp"
#include "RadarSensor.hpp"
#include "Spell.hpp"
#include "SpellUtil.hpp"
#include "value_map.hpp"

namespace ai{

OnDetect::OnDetect(StateMachine* fsm, GType type, AITargetFunctionGenerator gen) :
	Function(fsm),
	type(type),
	gen(gen)
{
}

event_bitset OnDetect::getEvents()
{
	return make_enum_bitfield(event_type::detect);
}

bool OnDetect::onEvent(Event event)
{
	if (event.getDetectType() == type && !thread) {
		thread = fsm->addThread(gen(fsm, any_cast<GObject*>(event.data)));
		return true;
	}
	return false;
}

WhileDetect::WhileDetect(StateMachine* fsm, GType type, AITargetFunctionGenerator gen) :
	Function(fsm),
	type(type),
	gen(gen)
{
}

event_bitset WhileDetect::getEvents()
{
	return enum_bitfield2(event_type, detect, endDetect);
}

bool WhileDetect::onEvent(Event event)
{
	if (event.getDetectType() == type) {
		if (!thread)
			thread = fsm->addThread(gen(fsm, any_cast<GObject*>(event.data)));

		return true;
	}
	else if (event.getEndDetectType() == type) {
		fsm->removeThread(thread);
		thread.reset();
		return true;
	}
	return false;
}

CompositeFunction::CompositeFunction(StateMachine* fsm) :
	Function(fsm)
{
}

void CompositeFunction::onEnter()
{
	for (auto f : functions) {
		f->onEnter();
	}
	hasInit = true;
}

update_return CompositeFunction::update()
{
	lock_mask locks;

	for (auto it = functions.rbegin(); it != functions.rend(); ++it) {
		lock_mask crntLock = (*it)->getLockMask();

		if ((crntLock & locks).any()) {
			continue;
		}

		locks &= crntLock;
		update_return result = (*it)->update();
		
		if (result.second) {
			log("CompositeFunction sub-function %s should not push!", (*it)->getName());
		}
		if (result.first < 0) {
			log("CompositeFunction sub-function %s should not pop!", (*it)->getName());
		}		
	}

	return_steady();
}

event_bitset CompositeFunction::getEvents()
{
	return events;
}

bool CompositeFunction::onEvent(Event event)
{
	for (auto it = functions.rbegin(); it != functions.rend(); ++it) {
		if ((*it)->onEvent(event)) {
			return true;
		}
	}
	return false;
}

void CompositeFunction::onExit()
{
	for (auto f : functions) {
		f->onExit();
	}
}

lock_mask CompositeFunction::getLockMask()
{
	lock_mask result;

	for (auto f : functions) {
		result &= f->getLockMask();
	}

	return result;
}

string CompositeFunction::getName()
{
	return "CompositeFunction";
}

void CompositeFunction::addFunction(shared_ptr<Function> f)
{
	if (hasInit) {
		f->onEnter();
	}
	functions.push_back(f);

	events &= f->getEvents();
}

void CompositeFunction::removeFunction(shared_ptr<Function> f)
{
	auto it = functions.begin();
	while (it != functions.end()) {
		if (*it == f) {
			if (hasInit) {
				f->onExit();
				functions.erase(it);
				return;
			}
		}
		else {
			++it;
		}
	}
}

BossFightHandler::BossFightHandler(StateMachine* fsm, string startDialog, string endDialog) :
	Function(fsm),
	startDialog(startDialog),
	endDialog(endDialog)
{
}
 
event_bitset BossFightHandler::getEvents()
{
	return enum_bitfield2(event_type, detect, zeroHP);
}

bool BossFightHandler::onEvent(Event event)
{
	if (event.isDetectPlayer() && !hasRunStart) {
		if (!startDialog.empty()) {
			fsm->getSpace()->createDialog(startDialog, false);
		}
		fsm->getObject()->getCrntRoom()->activateBossObjects();
		hasRunStart = true;
		return true;
	}

	if (event.eventType == event_type::zeroHP && !hasRunEnd) {
		if (!endDialog.empty()) {
			fsm->getSpace()->createDialog(endDialog, false);
		}
		fsm->getObject()->getCrntRoom()->deactivateBossObjects();
		hasRunEnd = true;
		return true;
	}

	return false;
}

Seek::Seek(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
    target = getObjRefFromStringField(fsm->getSpace(), args, "target_name");
	usePathfinding = getBoolOrDefault(args, "use_pathfinding", false);
}

Seek::Seek(StateMachine* fsm, GObject* target, bool usePathfinding, SpaceFloat margin) :
	Function(fsm),
	target(target),
	usePathfinding(usePathfinding),
	margin(margin)
{}

update_return Seek::update()
{
	if (target.isValid()) {
		if (usePathfinding && isObstacleBetweenTarget(agent, target.get())) {
			shared_ptr<Function> pathFunction = ai::PathToTarget::create(fsm, target.get());
			return_push_if_valid(pathFunction);
		}
		else if (distanceToTarget(agent, target.get()->getPos()) < margin) {
			arrive(agent, target.get()->getPos());
		}
		else {
			seek(
				agent,
				target.get()->getPos(),
				agent->getMaxSpeed(),
				agent->getMaxAcceleration()
			);
		}
		agent->setDirection(toDirection(
            ai::directionToTarget(
                agent,
                target.get()->getPos()
            )
        ));
	}

	return_pop_if_false(target.isValid());
}

ExplodeOnZeroHP::ExplodeOnZeroHP(StateMachine* fsm, DamageInfo damage, SpaceFloat radius) :
	Function(fsm),
	damage(damage),
	radius(radius)
{
}

bool ExplodeOnZeroHP::onEvent(Event event)
{
	if (event.eventType == event_type::zeroHP) {
		explode();
		return true;
	}
	return false;
}

event_bitset ExplodeOnZeroHP::getEvents()
{
	return make_enum_bitfield(event_type::zeroHP);
}

void ExplodeOnZeroHP::explode()
{
	GObject* obj = fsm->getObject();
	GSpace* space = getSpace();

	explosion(obj, radius, damage);

	SpriteID bombSprite = space->createSprite(
		&graphics_context::createSprite,
		string("sprites/explosion.png"),
		GraphicsLayer::overhead,
		toCocos(obj->getPos()) * app::pixelsPerTile,
		1.0f
	);
	space->graphicsNodeAction(
		&Node::setColor,
		bombSprite,
		Color3B::RED
	);
	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		bombSprite,
		bombAnimationAction(radius / Bomb::explosionSpriteRadius, true)
	);
	obj->playSoundSpatial("sfx/red_fairy_explosion.wav");

	LightID light = space->addLightSource(
		CircleLightArea::create(radius, Color4F::RED, 0.25),
		obj->getPos(),
		0.0
	);
	space->addGraphicsAction(&graphics_context::autoremoveLightSource, light, 1.0f);
}

MaintainDistance::MaintainDistance(StateMachine* fsm, gobject_ref target, SpaceFloat distance, SpaceFloat margin) :
Function(fsm),
target(target),
distance(distance),
margin(margin)
{}

MaintainDistance::MaintainDistance(StateMachine* fsm, const ValueMap& args) :
Function(fsm)
{
    target = getObjRefFromStringField(getSpace(), args, "target");
    distance = getFloat(args, "distance");
    margin = getFloat(args, "margin");
}

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

	return_steady();
}

const SpaceFloat Flock::separationDesired = 5.0;

Flock::Flock(StateMachine* fsm) :
	Function(fsm)
{
}

void Flock::onEnter()
{
}

update_return Flock::update()
{
	SpaceVect _separate = separate() * 1.5;
	SpaceVect _align = align() * 0.75;
	SpaceVect _cohesion = cohesion() * 0.75;

	SpaceVect sum = _separate + _align + _cohesion;

	agent->applyForceForSingleFrame(sum.setMag(agent->getMaxAcceleration()) );

	return_steady();
}

void Flock::onExit()
{
}

bool Flock::onEvent(Event event)
{
	if (event.getDetectType() == GType::enemy) {
		onDetectNeighbor(dynamic_cast<Agent*>(any_cast<GObject*>(event.data)));
		return true;
	}
	else if (event.getEndDetectType() == GType::enemy) {
		endDetectNeighbor(dynamic_cast<Agent*>(any_cast<GObject*>(event.data)));
		return true;
	}
	return false;
}

event_bitset Flock::getEvents()
{
	return enum_bitfield2(event_type, detect, endDetect);
}

void Flock::onDetectNeighbor(Agent* agent)
{
	neighbors.insert(agent);
}

void Flock::endDetectNeighbor(Agent* agent)
{
	neighbors.erase(agent);
}

SpaceVect Flock::separate()
{
	SpaceVect steer_acc;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();
		SpaceVect disp = agent->getPos() - other->getPos();

		//Actual magnitude added from interaction is 1/x.
		if (disp.lengthSq() < separationDesired*separationDesired) {
			steer_acc += disp.normalizeSafe() / disp.length();
			++count;
		}
	}

	if (count > 0) {
		steer_acc /= count;
		steer_acc = steer_acc.normalize() * agent->getMaxSpeed();
		steer_acc -= agent->getVel();
		steer_acc.limit(agent->getMaxAcceleration());

	}

	array<SpaceFloat, 8> walls = ai::wallFeeler8(agent, separationDesired);

	for (size_t i = 0; i < 8; ++i) {
		if (walls[i] < separationDesired) {
			SpaceVect v = SpaceVect::ray(1.0 / walls[i], i * float_pi / 4.0).rotate(float_pi);

			steer_acc += v;
			++count;
		}
	}

	return steer_acc;
}

//Calculate average velocity of neighbors
SpaceVect Flock::align()
{
	SpaceVect sum;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();

		sum += other->getVel();
		++count;
	}

	if (count > 0) {
		sum /= count;

		SpaceVect vel = sum.normalizeSafe() * agent->getMaxSpeed();
		SpaceVect steer = vel - agent->getVel();

		return steer.limit(agent->getMaxAcceleration());
	}
	else {
		return SpaceVect::zero;
	}
}

//Calculate average position of neighbors;
SpaceVect Flock::cohesion()
{
	SpaceVect sum;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();

		sum += other->getPos();
		++count;
	}

	if (count > 0) {
		return compute_seek(agent, sum / count);
	}
	else {
		return SpaceVect::zero;
	}
}

OccupyPoint::OccupyPoint(StateMachine* fsm, SpaceVect target) :
	Function(fsm),
	target(target)
{
}

update_return OccupyPoint::update()
{
	SpaceFloat crnt_distance = distanceToTarget(agent, target);
	SpaceFloat stop_dist = getStoppingDistance(agent);

	if (crnt_distance > stop_dist) {
		seek(agent, target, agent->getMaxSpeed(), agent->getMaxAcceleration());
	}
	else {
		arrive(agent, target);
	}

	return_steady();
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

	return_steady();
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
	if (!target.isValid() || endFrame != 0 && getSpace()->getFrame() >= endFrame) {
		return_pop();
	}

	SpaceVect displacement = displacementToTarget(agent, target.get()->getPos());

	SpaceFloat angle = displacement.toAngle();
	if (!scurryLeft) {
		angle += float_pi;
	}
	scurryLeft = !scurryLeft;

	array<SpaceFloat, 8> obstacleFeelers = obstacleFeeler8(agent, distance);
	int direction = chooseBestDirection(obstacleFeelers, angle, distance);

	if (direction != -1) {
		return_push(fsm->make<MoveToPoint>(
			agent->getPos() + SpaceVect::ray(distance, direction * float_pi / 4.0)
		));
	}
	else {
		return_steady();
	}
}

Flee::Flee(StateMachine* fsm, GObject* target, SpaceFloat distance) :
	Function(fsm),
	target(target),
	distance(distance)
{}


Flee::Flee(StateMachine* fsm, const ValueMap& args) : 
	Function(fsm)
{
    if(args.find("target_name") == args.end()){
        log("Seek::Seek: target_name missing.");
    }
    target = getSpace()->getObject(args.at("target_name").asString());
    
    if(!target.isValid()){
        log("Flee::Flee: target object %s not found.", args.at("target_name").asString().c_str() );
    }
    
    if(args.find("flee_distance") == args.end()){
        log("Flee::Flee: flee_distance missing.");
		distance = 0.0;
    }
    else{
        distance = args.at("flee_distance").asFloat();
    }
}

update_return Flee::update()
{
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
	
	return_pop_if_false(target.isValid());
}

EvadePlayerProjectiles::EvadePlayerProjectiles(StateMachine* fsm) : 
	Function(fsm)
{}

EvadePlayerProjectiles::EvadePlayerProjectiles(StateMachine* fsm, const ValueMap& args) : 
	Function(fsm)
{}

update_return EvadePlayerProjectiles::update()
{
	list<GObject*> objs = agent->getRadar()->getSensedObjectsByGtype(GType::playerBullet);
	
	GObject* closest = nullptr;
	SpaceFloat closestDistance = numeric_limits<SpaceFloat>::infinity();
	 
	for(GObject* obj: objs)
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

	return_steady();
}

IdleWait::IdleWait(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
    auto it = args.find("waitTime");
    
    if(it == args.end()){
        log("IdleWait::IdleWait: waitTime missing from ValueMap!");
        remaining = 0;
        return;
    }
    
    if(!it->second.isNumber()){
        log("IdleWait::IdleWait: waitTime is not a number!");
        remaining = 0;
        return;
    }

    if(it->second.asFloat() < 0.0f){
        log("IdleWait::IdleWait: waitTime is negative!");
        remaining = 0;
        return;
    }
    
    SpaceFloat waitSeconds = getFloat(args, "waitTime");
    remaining = app::params.framesPerSecond * waitSeconds;
}

IdleWait::IdleWait(StateMachine* fsm, int frames) :
	Function(fsm),
	remaining(frames)
{}

IdleWait::IdleWait(StateMachine* fsm) :
	Function(fsm),
	remaining(-1)
{}

update_return IdleWait::update()
{
	--remaining;
	ai::applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());

	return_pop_if_false(remaining > 0);
}

LookAround::LookAround(StateMachine* fsm, SpaceFloat angularVelocity) :
Function(fsm),
angularVelocity(angularVelocity)
{
}

update_return LookAround::update()
{
	agent->rotate(angularVelocity * app::params.secondsPerFrame);
	return_steady();
}

CircleAround::CircleAround(
	StateMachine* fsm,
	SpaceVect center,
	SpaceFloat startingAngularPos,
	SpaceFloat angularSpeed
) :
Function(fsm),
center(center),
angularPosition(startingAngularPos),
angularSpeed(angularSpeed)
{
}

void CircleAround::init()
{
}

update_return CircleAround::update()
{
	SpaceFloat radius = distanceToTarget(agent, center);
	SpaceFloat angleDelta = angularSpeed * app::params.secondsPerFrame;

	angularPosition += angleDelta;

	SpaceVect agentPos = center + SpaceVect::ray(radius, angularPosition);

	agent->setPos(agentPos);
	agent->setAngle(angularPosition);

	return_steady();
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
		return_push( fsm->make<MoveToPoint>(target_pos) );
	}
	else {
		applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}

	return_steady();
}


bool Flank::wallQuery(SpaceVect pos)
{
	return getPhys()->obstacleRadiusQuery(
		agent,
		pos, 
		wallMargin,
		GType::wall,
		PhysicsLayers::all
	);
}

QuadDirectionLookAround::QuadDirectionLookAround(
	StateMachine* fsm,
	boost::rational<int> secondsPerDirection,
	bool clockwise
) :
Function(fsm),
secondsPerDirection(secondsPerDirection),
timeRemaining(secondsPerDirection),
clockwise(clockwise)
{

}

update_return QuadDirectionLookAround::update()
{
	timerDecrement(timeRemaining);

	if (timeRemaining <= 0) {
		agent->rotate(float_pi / 2.0 * (clockwise ? 1.0 : -1.0));
		timeRemaining = secondsPerDirection;
	}

	return_steady();
}

AimAtTarget::AimAtTarget(StateMachine* fsm, gobject_ref target) :
Function(fsm),
target(target)
{
}

update_return AimAtTarget::update()
{
	if (!target.isValid())
		return_pop();

	agent->setAngle(directionToTarget(agent, target.get()->getPos()).toAngle());
	return_steady();
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

	return_steady();
}

void LookTowardsFire::onExit()
{
}

event_bitset LookTowardsFire::getEvents()
{
	return make_enum_bitfield(event_type::bulletHit);
}

bool LookTowardsFire::onEvent(Event event)
{
	if (event.eventType != event_type::bulletHit) {
		return false;
	}
	Bullet* b = any_cast<Bullet*>(event.data);

	SpaceVect bulletDirection = b->getVel().normalizeSafe().rotate(float_pi);
	hitAccumulator += hitCost;
	directionAccumulator += bulletDirection;

	if (looking) {
		agent->setAngle(bulletDirection.toAngle());
	}

	return true;
}

lock_mask LookTowardsFire::getLockMask()
{
	return looking ? (
		useShield ?
			enum_bitfield3(ResourceLock, movement, look, shield) :
			enum_bitfield2(ResourceLock, movement, look)
		) :
		lock_mask()
	;
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
	bool arrived = moveToPoint(agent, target, arrivalMargin, false);
	
	return_pop_if_true(arrived);
}

BezierMove::BezierMove(StateMachine* fsm, array<SpaceVect, 3> points, SpaceFloat rate) :
	Function(fsm),
	points(points),
	rate(rate)
{
}

update_return BezierMove::update()
{
	agent->setPos(bezier(points, t));

	timerIncrement(t, rate);
	if (t >= 1.0) {
		agent->setPos(bezier(points, 1.0));
		return_pop();
	}
	else
		return_steady();
}

PolarMove::PolarMove(StateMachine* fsm, SpaceFloat force, SpaceFloat angularSpeed) :
	Function(fsm),
	force(force),
	angularSpeed(angularSpeed)
{
}

update_return PolarMove::update()
{
	GObject* obj = fsm->getObject();
	obj->applyForceForSingleFrame(SpaceVect::ray(force, obj->getAngle() + float_pi / 2.0));
	obj->rotate(app::params.secondsPerFrame * angularSpeed);

	return_steady();
}

shared_ptr<FollowPath> FollowPath::pathToTarget(
	StateMachine* fsm,
	gobject_ref target
){
	if (!target.isValid()) {
		return nullptr;
	}
	Agent* agent = fsm->getAgent();

	return make_shared<FollowPath>(
		fsm,
		fsm->getSpace()->pathToTile(
			toIntVector(agent->getPos()),
			toIntVector(target.get()->getPos())
		),
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
	if (currentTarget < path.size()) {
		agent->setDirection(toDirection(ai::directionToTarget(agent, path[currentTarget])));
		bool arrived = moveToPoint(agent, path[currentTarget], MoveToPoint::arrivalMargin, stopForObstacle);
		currentTarget += arrived;
	}
	else if (loop && path.size() > 0) {
		currentTarget = 0;
	}
	else {
		return_pop();
	}

	return_steady();
}

shared_ptr<PathToTarget> PathToTarget::create(StateMachine* fsm, GObject* target)
{
	SpaceVect start = fsm->getAgent()->getPos();
	SpaceVect endpoint = target->getPos();

	Path p = fsm->getSpace()->pathToTile(
		toIntVector(start),
		toIntVector(endpoint)
	);

	if (p.empty()) {
		log("%s was unable to find path to target!", fsm->getAgent()->getProperName());
		return nullptr;
	}

	return make_shared<PathToTarget>(fsm, p, target);
}

PathToTarget::PathToTarget(StateMachine* fsm, Path path, gobject_ref target) :
	FollowPath(fsm, path, false, false),
	target(target)
{
}

update_return PathToTarget::update()
{
	if (ai::isObstacleBetweenTarget(agent, target.get())) {
		return_pop();
	}
	else {
		return FollowPath::update();
	}
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
	array<SpaceFloat, 4> feelers = ai::obstacleFeelerQuad(agent, maxDist);
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
	timerDecrement(waitTimer);

	if (waitTimer <= 0.0) {
		pair<Direction, SpaceFloat> movement = chooseMovement();

		if (movement.first != Direction::none && movement.second > 0.0) {
			agent->setDirection(movement.first);
			waitTimer = fsm->getSpace()->getRandomFloat(minWait, maxWait);
			return_push( fsm->make<MoveToPoint>(
				agent->getPos() + dirToVector(movement.first)*movement.second
			));
		}
	}
	else {
		applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}

	return_steady();
}

FireAtTarget::FireAtTarget(StateMachine* fsm, gobject_ref target) :
	Function(fsm),
	target(target)
{}

update_return FireAtTarget::update()
{
	FirePattern* fp = agent->getFirePattern();
	if (!target.isValid() || !fp) {
		return_pop();
	}

	agent->setAngle(
		directionToTarget(agent, target.get()->getPos()).toAngle()
	);

	if (fp->fireIfPossible()) {
		agent->playSoundSpatial("sfx/shot.wav");
	}

	return_steady();
}

FireIfTargetVisible::FireIfTargetVisible(StateMachine* fsm, gobject_ref target) :
	Function(fsm),
	target(target)
{}

update_return FireIfTargetVisible::update()
{
	FirePattern* fp = agent->getFirePattern();

	if (!fp || !target.isValid()) {
		return_pop();
	}
	
	if (agent->getRadar()->isObjectVisible(target.get()) && getSpace()->isInPlayerRoom(agent->getCrntRoomID()))
	{
		if (fp->fireIfPossible()) {
			agent->playSoundSpatial("sfx/shot.wav");
		}
	}

	return_steady();
}

Operation::Operation(StateMachine* fsm, std::function<void(StateMachine&)> op) :
	Function(fsm),
	op(op)
{}

update_return Operation::update() {
	op(*fsm);
	return_pop();
}

Cast::Cast(StateMachine* fsm, SpellGeneratorType spell_generator, SpaceFloat length) :
Function(fsm),
spell_generator(spell_generator),
length(length)
{
}

void Cast::onEnter()
{
	agent->cast(spell_generator(agent));
}

update_return Cast::update()
{
	timerIncrement(timer);

	if (length > 0.0 && timer >= length)
		agent->stopSpell();

	if (!agent->isSpellActive())
		return_pop();
	else
		return_steady();
}

void Cast::onExit()
{
	agent->stopSpell();
}

HPCast::HPCast(StateMachine* fsm, SpellGeneratorType spell_generator, float hp_difference) :
	Function(fsm),
	spell_generator(spell_generator),
	hp_difference(hp_difference)
{
}

void HPCast::onEnter()
{
	caster_starting = agent->getHealth();
	agent->cast(spell_generator(agent));
}

update_return HPCast::update()
{
	if (agent->getHealth() < (caster_starting - hp_difference)) {
		agent->stopSpell();
	}

	if (!agent->isSpellActive()) {
		return_pop();
	}
	else {
		return_steady();
	}
}

void HPCast::onExit()
{
	agent->stopSpell();
}

HPCastSequence::HPCastSequence(
	StateMachine* fsm,
	const vector<SpellGeneratorType>& spells,
	const boost::icl::interval_map<float, int> intervals
) :
	Function(fsm),
	spells(spells),
	intervals(intervals)
{
}

void HPCastSequence::onEnter()
{
	if (agent->isSpellActive()) {
		log("HPCastSequence::onEnter: %s already has spell active.", agent->getName());
		agent->stopSpell();
	}
}

update_return HPCastSequence::update()
{
	float hp = agent->getHealth();
	int newInterval = -1;

	auto it = intervals.find(hp);
	if (it != intervals.end()) {
		newInterval = it->second - 1;
	}

	if (newInterval != crntInterval && crntInterval != -1) {
		agent->stopSpell();
	}
	if (newInterval != crntInterval && newInterval != -1) {
		agent->cast(spells.at(newInterval)(agent));
	}
	crntInterval = newInterval;

	return_steady();
}

void HPCastSequence::onExit()
{
	if (crntInterval != -1) {
		agent->stopSpell();
	}
}

FireOnStress::FireOnStress(StateMachine* fsm, float stressPerShot) :
	Function(fsm),
	stressPerShot(stressPerShot)
{
}

update_return FireOnStress::update()
{
	if (agent->getAttribute(Attribute::stress) >= stressPerShot && agent->getFirePattern()->fireIfPossible()) {
		agent->modifyAttribute(Attribute::stress, -stressPerShot);
	}
	return_steady();
}

ThrowBombs::ThrowBombs(
	StateMachine* fsm,
	gobject_ref target,
	ParamsGeneratorType generator,
	SpaceFloat throwingSpeed,
	SpaceFloat baseInterval,
	SpaceFloat blastRadius,
	SpaceFloat fuseTime,
	float cost
) :
	Function(fsm),
	target(target),
	generator(generator),
	throwingSpeed(throwingSpeed),
	baseInterval(baseInterval),
	blastRadius(blastRadius),
	fuseTime(fuseTime),
	cost(cost)
{
}

void ThrowBombs::init()
{
	countdown = getInterval();
}

update_return ThrowBombs::update()
{
	if (!target.isValid()) {
		return_pop();
	}

	timerDecrement(countdown);

	if (countdown <= 0.0 && agent->getAttribute(Attribute::mp) >= cost) {
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
				bombObstacles,
				PhysicsLayers::ground
			) &&
			//bomb is likely to travel a significant distance
			getPhys()->obstacleDistanceFeeler(
				agent,
				SpaceVect::ray(1.0 + fuseTime*throwingSpeed, angle)
			) > blastRadius &&
			//predict net gain player-enemy damage
			score(agent->getPos(), angle) > 0.5f &&
			//do not throw bombs when fleeing
			!fsm->isThreadRunning("Flee") &&
			//use bombs gradually in a fight
			hasEnoughMagic &&
			//do not throw if target is too close
			distanceToTarget(agent, target.get()) > blastRadius
		) {
			getSpace()->createObject(generator(make_shared<object_params>(
				agent->getPos() + SpaceVect::ray(1.0, angle),
				agent->getVel() + SpaceVect::ray(throwingSpeed, angle)
			)));
			countdown = getInterval();
		}
	}
	return_steady();
}

SpaceFloat ThrowBombs::getInterval()
{
	return baseInterval / (*agent->getAttributeSystem())[Attribute::attackSpeed];
}

float ThrowBombs::score(SpaceVect pos, SpaceFloat angle)
{
	SpaceVect predictedPos = pos + SpaceVect::ray(1.0 + fuseTime * throwingSpeed, angle);
	return bombScore(getSpace(), predictedPos, blastRadius);
}

}//end NS
