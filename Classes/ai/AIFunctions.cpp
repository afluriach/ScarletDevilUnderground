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
#include "App.h"
#include "audio_context.hpp"
#include "Bullet.hpp"
#include "FirePattern.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"

namespace ai{

Seek::Seek(GSpace* space, const ValueMap& args) {
    target = getObjRefFromStringField(space, args, "target_name");
	usePathfinding = getBoolOrDefault(args, "use_pathfinding", false);
}

Seek::Seek(GObject* target, bool usePathfinding, SpaceFloat margin) :
	target(target),
	usePathfinding(usePathfinding),
	margin(margin)
{}

void Seek::update(StateMachine& sm)
{
	if (target.isValid()) {
		if (usePathfinding && isObstacle(sm.getAgent(), target.get()->getPos())) {
			sm.push(ai::PathToTarget::create(sm.agent, target.get()));
		}
		else if (distanceToTarget(sm.agent, target.get()->getPos()) < margin) {
			arrive(sm.agent, target.get()->getPos());
		}
		else {
			seek(
				sm.agent,
				target.get()->getPos(),
				sm.agent->getMaxSpeed(),
				sm.agent->getMaxAcceleration()
			);
		}
		sm.agent->setDirection(toDirection(
            ai::directionToTarget(
                sm.agent,
                target.get()->getPos()
            )
        ));
	}
	else{
		sm.getCrntThread()->pop();
    }
}

MaintainDistance::MaintainDistance(gobject_ref target, SpaceFloat distance, SpaceFloat margin) :
target(target),
distance(distance),
margin(margin)
{}

MaintainDistance::MaintainDistance(GSpace* space, const ValueMap& args)
{
    target = getObjRefFromStringField(space, args, "target");
    distance = getFloat(args, "distance");
    margin = getFloat(args, "margin");
}

void MaintainDistance::update(StateMachine& sm)
{
	if (target.get()) {
        SpaceFloat crnt_distance = distanceToTarget(sm.agent,target.get());
		SpaceFloat stop_dist = getStoppingDistance(sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
    
		if (abs(crnt_distance - distance) < stop_dist) {
			ai::arrive(
				sm.agent,
				SpaceVect::ray(distance, float_pi + directionToTarget(sm.agent, target.get()->getPos()).toAngle())
			);
		}

        else if(crnt_distance > distance + margin){
            ai::seek(
                sm.agent,
                target.get()->getPos(),
                sm.agent->getMaxSpeed(),
                sm.agent->getMaxAcceleration()
            );
        }
        else if(crnt_distance < distance + margin){
            ai::fleeWithObstacleAvoidance(
                sm.agent,
                target.get()->getPos(),
                sm.agent->getMaxSpeed(),
                sm.agent->getMaxAcceleration()
            );
        }
	}
	else {
		ai::applyDesiredVelocity(sm.agent, SpaceVect::zero, sm.agent->getMaxAcceleration());
	}
}

const SpaceFloat Flock::separationDesired = 5.0;

Flock::Flock()
{
}

void Flock::update(StateMachine& sm)
{
	SpaceVect _separate = separate(sm.getAgent()) * 1.5;
	SpaceVect _align = align(sm.getAgent()) * 0.75;
	SpaceVect _cohesion = cohesion(sm.getAgent()) * 0.75;

	SpaceVect sum = _separate + _align + _cohesion;

	sm.agent->applyForceForSingleFrame(sum.setMag(sm.getAgent()->getMaxAcceleration()) );
}

void Flock::onDetectNeighbor(Agent* agent)
{
	neighbors.insert(agent);
}

void Flock::endDetectNeighbor(Agent* agent)
{
	neighbors.erase(agent);
}

SpaceVect Flock::separate(Agent* _agent)
{
	SpaceVect steer_acc;
	int count = 0;

	for (auto ref : neighbors)
	{
		Agent* other = ref.get();
		SpaceVect disp = _agent->getPos() - other->getPos();

		//Actual magnitude added from interaction is 1/x.
		if (disp.lengthSq() < separationDesired*separationDesired) {
			steer_acc += disp.normalizeSafe() / disp.length();
			++count;
		}
	}

	if (count > 0) {
		steer_acc /= count;
		steer_acc = steer_acc.normalize() * _agent->getMaxSpeed();
		steer_acc -= _agent->getVel();
		steer_acc.limit(_agent->getMaxAcceleration());

	}

	array<SpaceFloat, 8> walls = ai::wallFeeler8(_agent, separationDesired);

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
SpaceVect Flock::align(Agent* _agent)
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

		SpaceVect vel = sum.normalizeSafe() * _agent->getMaxSpeed();
		SpaceVect steer = vel - _agent->getVel();

		return steer.limit(_agent->getMaxAcceleration());
	}
	else {
		return SpaceVect::zero;
	}
}

//Calculate average position of neighbors;
SpaceVect Flock::cohesion(Agent* _agent)
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
		return compute_seek(_agent, sum / count);
	}
	else {
		return SpaceVect::zero;
	}
}

OccupyPoint::OccupyPoint(SpaceVect target) :
	target(target)
{
}

void OccupyPoint::update(StateMachine& sm)
{
	SpaceFloat crnt_distance = distanceToTarget(sm.agent, target);
	SpaceFloat stop_dist = getStoppingDistance(sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());

	if (crnt_distance > stop_dist) {
		seek(sm.agent, target, sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
	}
	else {
		arrive(sm.agent, target);
	}
}

OccupyMidpoint::OccupyMidpoint(gobject_ref target1, gobject_ref target2) :
target1(target1),
target2(target2)
{
}

void OccupyMidpoint::update(StateMachine& sm)
{
	GObject* t1 = target1.get();
	GObject* t2 = target2.get();

	if (!t1 || !t2) {
		sm.pop();
		return;
	}

	SpaceVect midpoint = (t1->getPos() + t2->getPos()) / 2.0;
	SpaceFloat crnt_distance = distanceToTarget(sm.agent, midpoint);
	SpaceFloat stop_dist = getStoppingDistance(sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());

	if (crnt_distance > stop_dist) {
		seek(sm.agent, midpoint, sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
	}
	else {
		arrive(sm.agent, midpoint);
	}
}

Scurry::Scurry(GSpace* space, GObject* _target, SpaceFloat _distance, SpaceFloat length) :
distance(_distance),
target(_target)
{
	startFrame = space->getFrame();

	if (length > 0.0)
		endFrame = startFrame + App::framesPerSecond*length;
	else
		endFrame = 0;
}

void Scurry::update(StateMachine& sm)
{
	if (!target.isValid() || endFrame != 0 && sm.agent->space->getFrame() >= endFrame) {
		sm.pop();
	}

	SpaceVect displacement = displacementToTarget(sm.agent, target.get()->getPos());

	SpaceFloat angle = displacement.toAngle();
	if (!scurryLeft) {
		angle += float_pi;
	}
	scurryLeft = !scurryLeft;

	array<SpaceFloat, 8> obstacleFeelers = obstacleFeeler8(sm.agent, distance);
	int direction = chooseBestDirection(obstacleFeelers, angle, distance);

	if (direction != -1) {
		sm.push(make_shared <MoveToPoint>(
			sm.agent->getPos() + SpaceVect::ray(distance, direction * float_pi / 4.0)
		));
	}
}

Flee::Flee(GObject* target, SpaceFloat distance) :
	target(target),
	distance(distance)
{}


Flee::Flee(GSpace* space, const ValueMap& args) {
    if(args.find("target_name") == args.end()){
        log("Seek::Seek: target_name missing.");
    }
    target = space->getObject(args.at("target_name").asString());
    
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

void Flee::update(StateMachine& sm)
{
	if (target.isValid()) {
		ai::fleeWithObstacleAvoidance(
			sm.agent,
			target.get()->getPos(),
			sm.agent->getMaxSpeed(),
			sm.agent->getMaxAcceleration()
		);
		sm.agent->setDirection(toDirection(
            ai::directionToTarget(
                sm.agent,
                target.get()->getPos()
            )
        ));
	}
	else{
        sm.getCrntThread()->pop();
    }

}

EvadePlayerProjectiles::EvadePlayerProjectiles() {}

EvadePlayerProjectiles::EvadePlayerProjectiles(GSpace* space, const ValueMap& args) {}

void EvadePlayerProjectiles::update(StateMachine& sm)
{
	list<GObject*> objs = sm.getAgent()->getSensedObjectsByGtype(GType::playerBullet);
	
	GObject* closest = nullptr;
	SpaceFloat closestDistance = numeric_limits<SpaceFloat>::infinity();
	 
	for(GObject* obj: objs)
	{
		SpaceFloat crntDist = distanceToTarget(obj, sm.agent);

		if (crntDist < closestDistance) {
			closestDistance = crntDist;
			closest = obj;
		}
	}

	active = closest != nullptr;

	if (closest != nullptr)
	{
		SpaceVect offset = projectileEvasion(closest, sm.agent);
		if(!offset.isZero())
			applyDesiredVelocity(sm.agent, offset.normalize()*-1.0f * sm.agent->getMaxSpeed(), sm.agent->getMaxAcceleration());
	}
}

IdleWait::IdleWait(GSpace* space, const ValueMap& args)
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
    remaining = App::framesPerSecond * waitSeconds;
}

IdleWait::IdleWait(int frames) :
	remaining(frames)
{}

IdleWait::IdleWait() :
	remaining(-1)
{}

void IdleWait::update(StateMachine& fsm)
{
	if (remaining == 0)
		fsm.getCrntThread()->pop();
	--remaining;

	ai::applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.agent->getMaxAcceleration());
}

LookAround::LookAround(SpaceFloat angularVelocity) :
angularVelocity(angularVelocity)
{
}

void LookAround::update(StateMachine& fsm)
{
	fsm.agent->rotate(angularVelocity * App::secondsPerFrame);
}

CircleAround::CircleAround(SpaceVect center, SpaceFloat startingAngularPos, SpaceFloat angularSpeed) :
center(center),
angularPosition(startingAngularPos),
angularSpeed(angularSpeed)
{
}

void CircleAround::init(StateMachine& fsm)
{
}

void CircleAround::update(StateMachine& fsm)
{
	SpaceFloat radius = distanceToTarget(fsm.agent, center);
	SpaceFloat angleDelta = angularSpeed * App::secondsPerFrame;

	angularPosition += angleDelta;

	SpaceVect agentPos = center + SpaceVect::ray(radius, angularPosition);

	fsm.agent->setPos(agentPos);
	fsm.agent->setAngle(angularPosition);
}

Flank::Flank(gobject_ref target, SpaceFloat desiredDistance, SpaceFloat wallMargin) :
target(target),
desiredDistance(desiredDistance),
wallMargin(wallMargin)
{
}

void Flank::init(StateMachine& fsm)
{
}

void Flank::update(StateMachine& fsm)
{
	if (!target.isValid()) {
		fsm.pop();
	}

	SpaceVect target_pos;
	SpaceVect pos = target.get()->getPos();
	SpaceFloat angle = target.get()->getAngle();
	SpaceFloat this_angle = viewAngleToTarget(target.get(), fsm.agent);

	SpaceVect rear_pos = SpaceVect::ray(desiredDistance, angle + float_pi) + pos;
	SpaceVect left_pos = SpaceVect::ray(desiredDistance, angle - float_pi / 2.0) + pos;
	SpaceVect right_pos = SpaceVect::ray(desiredDistance, angle + float_pi / 2.0) + pos;

	if (abs(this_angle) < float_pi / 4.0) {
		//move to side flank

		if (this_angle < 0 && !wallQuery(fsm, left_pos)) {
			target_pos = left_pos;
		}
		else if(!wallQuery(fsm, right_pos)) {
			target_pos = right_pos;
		}
	}
	else if(!wallQuery(fsm, rear_pos))
	{
		//move to rear flank
		target_pos = rear_pos;
	}
	if (!target_pos.isZero()) {
		fsm.push(make_shared<MoveToPoint>(target_pos));
	}
	else {
		applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.agent->getMaxAcceleration());
	}
}


bool Flank::wallQuery(StateMachine& fsm, SpaceVect pos)
{
	return fsm.agent->space->obstacleRadiusQuery(fsm.agent, pos, wallMargin, GType::wall, PhysicsLayers::all);
}

QuadDirectionLookAround::QuadDirectionLookAround(boost::rational<int> secondsPerDirection, bool clockwise) :
secondsPerDirection(secondsPerDirection),
timeRemaining(secondsPerDirection),
clockwise(clockwise)
{

}

void QuadDirectionLookAround::update(StateMachine& fsm)
{
	timerDecrement(timeRemaining);

	if (timeRemaining <= 0) {
		fsm.agent->rotate(float_pi / 2.0 * (clockwise ? 1.0 : -1.0));
		timeRemaining = secondsPerDirection;
	}
}

AimAtTarget::AimAtTarget(gobject_ref target) :
target(target)
{
}

void AimAtTarget::update(StateMachine& fsm)
{
	if (!target.isValid())
		return;

	fsm.agent->setAngle(directionToTarget(fsm.agent, target.get()->getPos()).toAngle());
}

LookTowardsFire::LookTowardsFire(bool useShield) :
	useShield(useShield)
{
}

void LookTowardsFire::onEnter(StateMachine& fsm)
{
	hitCallbackID = fsm.addBulletHitFunction(bind(&LookTowardsFire::onBulletCollide, this, placeholders::_1, placeholders::_2));
	blockCallbackID = fsm.addBulletBlockFunction(bind(&LookTowardsFire::onBulletCollide, this, placeholders::_1, placeholders::_2));
}

void LookTowardsFire::update(StateMachine& fsm)
{
	hitAccumulator -= (looking*lookTimeCoeff + (1-looking)*timeCoeff)* App::secondsPerFrame;
	hitAccumulator = max(hitAccumulator, 0.0f);

	if (hitAccumulator == 0.0f) {
		directionAccumulator = SpaceVect::zero;
		looking = false;
		if (useShield) fsm.getAgent()->setShieldActive(false);
	}
	else if (!looking && hitAccumulator >= 1.0f) {
		fsm.agent->setAngle(directionAccumulator.toAngle());
		looking = true;
		if (useShield) fsm.getAgent()->setShieldActive(true);
	}

	if (looking) {
		applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.getAgent()->getMaxAcceleration());
	}
}

void LookTowardsFire::onExit(StateMachine& fsm)
{
	fsm.removeBulletFunction(hitCallbackID);
	fsm.removeBulletFunction(blockCallbackID);
}

void LookTowardsFire::onBulletCollide(StateMachine& fsm, Bullet* b)
{
	SpaceVect bulletDirection = b->getVel().normalizeSafe().rotate(float_pi);
	hitAccumulator += hitCost;
	directionAccumulator += bulletDirection;

	if (looking) {
		fsm.agent->setAngle(bulletDirection.toAngle());
	}
}

bitset<lockCount> LookTowardsFire::getLockMask()
{
	return looking ? (
		useShield ?
			enum_bitfield3(ResourceLock, movement, look, shield) :
			enum_bitfield2(ResourceLock, movement, look)
		) :
		bitset<lockCount>()
	;
}

const double MoveToPoint::arrivalMargin = 0.125;

MoveToPoint::MoveToPoint(GSpace* space, const ValueMap& args)
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

MoveToPoint::MoveToPoint(SpaceVect target) :
	target(target)
{}

void MoveToPoint::update(StateMachine& fsm)
{
	bool arrived = moveToPoint(fsm.agent, target, arrivalMargin, false);
	
	if (arrived) {
		fsm.pop();
	}
}

BezierMove::BezierMove(array<SpaceVect, 3> points, SpaceFloat rate) :
	points(points),
	rate(rate)
{
}

void BezierMove::update(StateMachine& fsm)
{
	fsm.agent->setPos(bezier(points, t));

	timerIncrement(t, rate);
	if (t >= 1.0) {
		fsm.agent->setPos(bezier(points, 1.0));
		fsm.pop();
	}
}

shared_ptr<FollowPath> FollowPath::pathToTarget(GSpace* space, gobject_ref agent, gobject_ref target)
{
	if (!agent.isValid() || !target.isValid()) {
		return nullptr;
	}

	return make_shared<ai::FollowPath>(
		space->pathToTile(
			toIntVector(agent.get()->getPos()),
			toIntVector(target.get()->getPos())
		),
		false,
		false
	);
}

FollowPath::FollowPath(Path path, bool loop, bool stopForObstacle) :
	path(path),
	loop(loop),
	stopForObstacle(stopForObstacle)
{}

FollowPath::FollowPath(GSpace* space, const ValueMap& args)
{
	auto name_it = args.find("pathName");
	auto loop_it = args.find("loop");

	if (name_it == args.end()) {
		log("FollowPath: pathName not provided!");
	}

	Path const* p = space->getPath(name_it->second.asString());

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

void FollowPath::update(StateMachine&  fsm)
{
	if (currentTarget < path.size()) {
		fsm.agent->setDirection(toDirection(ai::directionToTarget(fsm.agent, path[currentTarget])));
		bool arrived = moveToPoint(fsm.agent, path[currentTarget], MoveToPoint::arrivalMargin, stopForObstacle);
		currentTarget += arrived;
	}
	else if (loop && path.size() > 0) {
		currentTarget = 0;
	}
	else {
		fsm.pop();
	}
}

shared_ptr<PathToTarget> PathToTarget::create(GObject* agent, GObject* target)
{
	Path p = agent->space->pathToTile(
		toIntVector(agent->getPos()),
		toIntVector(target->getPos())
	);

	return make_shared<PathToTarget>(p, target);
}

PathToTarget::PathToTarget(Path path, gobject_ref target) :
	FollowPath(path, false, false),
	target(target)
{
}

void PathToTarget::update(StateMachine& fsm)
{
	if (ai::isLineOfSight(fsm.agent, target.get())) {
		fsm.pop();
	}
	else {
		FollowPath::update(fsm);
	}
}

Wander::Wander(GSpace* space, const ValueMap& args) :
    init_float_field(minWait,1.0f),
    init_float_field(maxWait,1.0f),
    init_float_field(minDist,1.0f),
    init_float_field(maxDist,1.0f)
{}

Wander::Wander(SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist) :
	minWait(minWait),
	maxWait(maxWait),
	minDist(minDist),
	maxDist(maxDist)
{
}

Wander::Wander(SpaceFloat waitInterval, SpaceFloat moveDist) :
	Wander(waitInterval, waitInterval, moveDist, moveDist)
{}

Wander::Wander() : 
	Wander(1.0, 1.0, 1.0, 1.0)
{}

pair<Direction, SpaceFloat> Wander::chooseMovement(StateMachine& fsm)
{
	array<SpaceFloat, 4> feelers = ai::obstacleFeelerQuad(fsm.agent, maxDist);
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
		return make_pair(directions.at(fsm.agent->space->getRandomInt(0, directions.size()-1)), maxDist);
	}

	//Select a direction that allows at least minimum desired distance.
	enum_foreach(Direction, d, right, end)
	{
		if (feelers[to_int(d) - 1] >= minDist) {
			directions.push_back(d);
		}
	}

	if (!directions.empty()) {
		int idx = fsm.agent->space->getRandomInt(0, directions.size() - 1);
		return make_pair(directions.at(idx), feelers[idx]);
	}

	return make_pair(Direction::none, 0.0);
}

void Wander::update(StateMachine& fsm)
{
	timerDecrement(waitTimer);

	if (waitTimer <= 0.0) {
		pair<Direction, SpaceFloat> movement = chooseMovement(fsm);

		if (movement.first != Direction::none && movement.second > 0.0) {
			fsm.agent->setDirection(movement.first);
			fsm.push(make_shared<MoveToPoint>(
				fsm.agent->getPos() + dirToVector(movement.first)*movement.second
			));
			waitTimer = fsm.agent->space->getRandomFloat(minWait, maxWait);
		}
	}
	else {
		applyDesiredVelocity(fsm.agent, SpaceVect::zero, fsm.agent->getMaxAcceleration());
	}
}

FireAtTarget::FireAtTarget(gobject_ref target) :
	target(target)
{}

void FireAtTarget::update(StateMachine& sm)
{
	FirePattern* fp = sm.getAgent()->getFirePattern();
	if (!target.isValid() || !fp) {
		sm.pop();
		return;
	}

	sm.agent->setAngle(
		directionToTarget(sm.agent, target.get()->getPos()).toAngle()
	);

	if (fp->fireIfPossible()) {
		sm.agent->space->audioContext->playSoundSpatial(
			"sfx/shot.wav",
			toVec3(sm.agent->getPos()), 
			toVec3(sm.agent->getVel())
		);
	}
}

FireIfTargetVisible::FireIfTargetVisible(gobject_ref target) :
	target(target)
{}

void FireIfTargetVisible::update(StateMachine& sm)
{
	RadarObject* ro = dynamic_cast<RadarObject*>(sm.agent);
	FirePattern* fp = sm.getAgent()->getFirePattern();

	if (!ro || !fp || !target.isValid()) {
		sm.pop();
		return;
	}
	
	if (ro->isObjectVisible(target.get()) && sm.agent->space->isInPlayerRoom(sm.agent->getPos()))
	{
		if (fp->fireIfPossible()) {
			sm.agent->space->audioContext->playSoundSpatial(
				"sfx/shot.wav",
				toVec3(sm.agent->getPos()),
				toVec3(sm.agent->getVel())
			);
		}
	}
}

Operation::Operation(std::function<void(StateMachine&)> op) :
	op(op)
{}

void Operation::update(StateMachine& sm) {
	op(sm);
	sm.pop();
}

Cast::Cast(SpellGeneratorType spell_generator, SpaceFloat length) :
spell_generator(spell_generator),
length(length)
{
}

void Cast::onEnter(StateMachine& sm)
{
	sm.agent->cast(spell_generator(sm.agent));
}

void Cast::update(StateMachine& sm)
{
	timerIncrement(timer);

	if (length > 0.0 && timer >= length)
		sm.agent->stopSpell();

	if (!sm.agent->isSpellActive())
		sm.pop();
}

void Cast::onExit(StateMachine& sm)
{
	sm.agent->stopSpell();
}

HPCast::HPCast(SpellGeneratorType spell_generator, float hp_difference) :
	spell_generator(spell_generator),
	hp_difference(hp_difference)
{
}

void HPCast::onEnter(StateMachine& sm)
{
	caster_starting = sm.getAgent()->getHealth();
	sm.agent->cast(spell_generator(sm.agent));
}

void HPCast::update(StateMachine& sm)
{
	if (sm.getAgent()->getHealth() < (caster_starting - hp_difference)) {
		sm.agent->stopSpell();
	}

	if (!sm.agent->isSpellActive()) {
		sm.pop();
	}
}

void HPCast::onExit(StateMachine& sm)
{
	sm.agent->stopSpell();
}

HPCastSequence::HPCastSequence(const vector<SpellGeneratorType>& spells, const boost::icl::interval_map<float, int> intervals) :
	spells(spells),
	intervals(intervals)
{
}

void HPCastSequence::onEnter(StateMachine& sm)
{
	if (sm.agent->isSpellActive()) {
		log("HPCastSequence::onEnter: %s already has spell active.", sm.agent->name.c_str());
		sm.agent->stopSpell();
	}
}

void HPCastSequence::update(StateMachine& sm)
{
	float hp = sm.getAgent()->getHealth();
	int newInterval = -1;

	auto it = intervals.find(hp);
	if (it != intervals.end()) {
		newInterval = it->second - 1;
	}

	if (newInterval != crntInterval && crntInterval != -1) {
		sm.agent->stopSpell();
	}
	if (newInterval != crntInterval && newInterval != -1) {
		sm.agent->cast(spells.at(newInterval)(sm.agent));
	}
	crntInterval = newInterval;
}

void HPCastSequence::onExit(StateMachine& sm)
{
	if (crntInterval != -1) {
		sm.agent->stopSpell();
	}
}

FireOnStress::FireOnStress(float stressPerShot) :
	stressPerShot(stressPerShot)
{
}

void FireOnStress::update(StateMachine& sm)
{
	Agent* a = sm.getAgent();

	if (a->getAttribute(Attribute::stress) >= stressPerShot && a->getFirePattern()->fireIfPossible()) {
		a->modifyAttribute(Attribute::stress, -stressPerShot);
	}
}

ThrowBombs::ThrowBombs(
	gobject_ref target,
	BombGeneratorType generator,
	SpaceFloat throwingSpeed,
	SpaceFloat baseInterval,
	SpaceFloat blastRadius,
	SpaceFloat fuseTime,
	float cost
) :
	target(target),
	generator(generator),
	throwingSpeed(throwingSpeed),
	baseInterval(baseInterval),
	blastRadius(blastRadius),
	fuseTime(fuseTime),
	cost(cost)
{
}

void ThrowBombs::init(StateMachine& fsm)
{
	countdown = getInterval(fsm);
}

void ThrowBombs::update(StateMachine& fsm)
{
	if (!target.isValid()) {
		fsm.pop();
		return;
	}

	timerDecrement(countdown);

	if (countdown <= 0.0 && fsm.getAgent()->getAttribute(Attribute::mp) >= cost) {
		SpaceFloat angle = directionToTarget(fsm.agent, target.get()->getPos()).toAngle();
		SpaceVect pos = fsm.agent->getPos() + SpaceVect::ray(1.0, angle);
		SpaceVect vel = fsm.agent->getVel() + SpaceVect::ray(throwingSpeed, angle);
		bool hasEnoughMagic =
			fsm.getAgent()->getAttributeSystem()->getHealthRatio() <
			fsm.getAgent()->getAttributeSystem()->getMagicRatio()
		;

		if (
			//can place bomb
			!fsm.agent->space->obstacleRadiusQuery(fsm.agent, pos, 0.5, bombObstacles, PhysicsLayers::ground) &&
			//bomb is likely to travel a significant distance
			fsm.agent->space->obstacleDistanceFeeler(fsm.agent, SpaceVect::ray(1.0 + fuseTime*throwingSpeed, angle)) > blastRadius &&
			//predict net gain player-enemy damage
			score(fsm.agent->space, fsm.agent->getPos(), angle) > 0.5f &&
			//do not throw bombs when fleeing
			!fsm.isThreadRunning("Flee") &&
			//use bombs gradually in a fight
			hasEnoughMagic &&
			//do not throw if target is too close
			distanceToTarget(fsm.agent, target.get()) > blastRadius
		) {
			fsm.agent->space->createObject(generator(
				fsm.agent->getPos() + SpaceVect::ray(1.0, angle),
				fsm.agent->getVel() + SpaceVect::ray(throwingSpeed, angle)
			));
			countdown = getInterval(fsm);
		}
	}
}

SpaceFloat ThrowBombs::getInterval(StateMachine& fsm)
{
	return baseInterval / (*fsm.getAgent()->getAttributeSystem())[Attribute::attackSpeed];
}

float ThrowBombs::score(GSpace* space, SpaceVect pos, SpaceFloat angle)
{
	SpaceVect predictedPos = pos + SpaceVect::ray(1.0 + fuseTime * throwingSpeed, angle);
	return bombScore(space, predictedPos, blastRadius);
}

}//end NS
