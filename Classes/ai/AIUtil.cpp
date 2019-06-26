//
//  AIUtil.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AI.hpp"
#include "AIUtil.hpp"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "physics_context.hpp"
#include "SpellUtil.hpp"

namespace ai{

//It seems to better to base this on acceleration than force. In some cases mass may
//just be a default value, in which case acceleration makes more sense.
void applyDesiredVelocity(GObject* obj, SpaceVect desired, SpaceFloat acceleration)
{
	SpaceVect floorVel = obj->getFloorVelocity();
	SpaceFloat traction = obj->getTraction();

	desired += floorVel;
	acceleration *= traction;

    //the desired velocity change
    SpaceVect vv = desired - obj->getVel();
    //the scalar amount of velocity change in one frame
	SpaceFloat dv = acceleration * app::params.secondsPerFrame;

    //Default case, apply maximum acceleration
    if(square_expr(dv) < vv.lengthSq()){
		SpaceFloat f = obj->getMass() * acceleration;
        SpaceVect ff = f * vv.normalizeSafe();
        obj->applyForceForSingleFrame(ff);
    }
    else{
        obj->setVel(desired);
    }
}

bool isFacingTarget(const GObject* agent, const GObject* target)
{
    SpaceVect targetDirection = (target->getPos() - agent->getPos()).normalize();
    SpaceVect agentFacingVector = agent->getFacingVector();
    bool facing = SpaceVect::dot(agentFacingVector, target->getFacingVector()) < 0;
    bool targetInFrontOfAgent = SpaceVect::dot(agentFacingVector, targetDirection) > 0;

    return facing && targetInFrontOfAgent;
}

bool isFacingTargetsBack(const GObject* agent, const GObject* target)
{
    SpaceVect targetDirection = (target->getPos() - agent->getPos()).normalize();
    SpaceVect agentFacingVector = agent->getFacingVector();

    bool facingBack = SpaceVect::dot(agentFacingVector, target->getFacingVector()) > 0;
    bool targetInFrontOfAgent = SpaceVect::dot(agentFacingVector, targetDirection) > 0;
    
    return facingBack && targetInFrontOfAgent;
}

bool isLineOfSight(const GObject* agent, const GObject* target)
{
    return agent->space->physicsContext->lineOfSight(agent, target);
}

array<SpaceFloat, 4> obstacleFeelerQuad(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 4> results;

	for (int i = 0; i < 4; ++i)
	{
		SpaceVect feeler = dirToVector(static_cast<Direction>(i+1)) * distance;
		results[i] = agent->space->physicsContext->obstacleDistanceFeeler(
			agent,
			feeler,
			agent->getRadius()*2.0
		);
	}

	return results;
}

array<SpaceFloat, 8> obstacleFeeler8(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 8> results;

	for (int i = 0; i < 8; ++i)
	{
		SpaceVect feeler = SpaceVect::ray(distance, i* float_pi / 4.0);
		results[i] = agent->space->physicsContext->obstacleDistanceFeeler(
			agent,
			feeler,
			agent->getRadius()*2.0
		);
	}

	return results;
}

array<SpaceFloat, 8> wallFeeler8(const GObject* agent, SpaceFloat distance)
{
	array<SpaceFloat, 8> results;

	for (int i = 0; i < 8; ++i)
	{
		SpaceVect feeler = SpaceVect::ray(distance, i* float_pi / 4.0);
		results[i] = agent->space->physicsContext->wallDistanceFeeler(agent, feeler);
	}

	return results;
}

int chooseBestDirection(const array<SpaceFloat, 8>& feelers, SpaceFloat desired_angle, SpaceFloat min_distance)
{
	int bestDirection = -1;
	SpaceFloat bestAngleDistance = float_pi;

	for (int i = 0; i < 8; ++i)
	{
		if (feelers[i] >= min_distance && abs(float_pi * i / 4.0 - desired_angle) < bestAngleDistance)
		{
			bestDirection = i;
			bestAngleDistance = abs(float_pi * i / 4.0 - desired_angle);
		}
	}

	return bestDirection;
}

//Trajectory represents direction/velocity of movement, but scaled to the same length as displacement.
//The difference between these vectors is the distance between the two centers of the objects,
//and thus an indicator of whether a collision will happen, or how the agent needs to move to prevent this.
SpaceVect projectileEvasion(const GObject* bullet, const GObject* agent)
{
	SpaceVect displacementToTarget = agent->getPos() - bullet->getPos();

	SpaceVect trajectoryScaled = bullet->getVel().normalizeSafe() * displacementToTarget.length();

	return trajectoryScaled - displacementToTarget;
}

SpaceVect directionToTarget(const GObject* agent, SpaceVect target)
{
    return (target - agent->getPos()).normalize();
}

SpaceVect displacementToTarget(const GObject* agent, SpaceVect target)
{
    return target - agent->getPos();
}

SpaceFloat distanceToTarget(const GObject* agent, const GObject* target)
{
	return distanceToTarget(agent->getPos(), target->getPos());
}

SpaceFloat distanceToTarget(const GObject* agent, SpaceVect target)
{
	return distanceToTarget(agent->getPos(), target);
}

SpaceFloat distanceToTarget(SpaceVect pos, SpaceVect target)
{
	return (target - pos).length();
}

SpaceFloat viewAngleToTarget(const GObject* agent, const GObject* target)
{
    SpaceVect displacement = target->getPos() - agent->getPos();
    
    if(displacement.lengthSq() > 0.01)
        return canonicalAngle(displacement.toAngle() - agent->getAngle());
    else
        return numeric_limits<SpaceFloat>::infinity();
}

bool isInFieldOfView(GObject* agent, SpaceVect target, SpaceFloat fovAngleScalarProduct)
{
	SpaceVect facingUnit = SpaceVect::ray(1.0, agent->getAngle());
	SpaceVect displacementUnit = (target - agent->getPos()).normalize();
	SpaceFloat scalar = SpaceVect::dot(facingUnit, displacementUnit);

	return scalar >= fovAngleScalarProduct;
}

bool isObstacle(Agent* agent, SpaceVect target)
{
	SpaceFloat maxSpeed = agent->getMaxSpeed();
	SpaceFloat acceleration = agent->getMaxAcceleration();
	SpaceVect displacement = compute_seek(agent, target).normalizeSafe();
	SpaceFloat distanceMargin = getTurningRadius(agent->getVel().length(), acceleration) + agent->getRadius();

	SpaceFloat dist = agent->space->physicsContext->obstacleDistanceFeeler(
		agent,
		displacement * distanceMargin,
		agent->getRadius()*2.0
	);
	
	return dist < distanceMargin;
}

bool isObstacleBetweenTarget(Agent* agent, const GObject* target)
{
	return agent->space->physicsContext->obstacleToTarget(
		agent,
		target,
		agent->getRadius() * 2.0
	);
}

SpaceVect compute_seek(Agent* agent, SpaceVect target)
{
	SpaceVect displacement = target - agent->getPos();

	if (displacement.lengthSq() < 1e-4)
		return SpaceVect::zero;

	SpaceVect direction = directionToTarget(agent, target);

	SpaceVect v = direction*agent->getMaxSpeed();
	return (v - agent->getVel()).limit(agent->getMaxAcceleration());
}

void seek(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration)
{
	SpaceVect displacement = target - agent->getPos();

	if (displacement.lengthSq() < 1e-4)
		return;

    SpaceVect direction = directionToTarget(agent,target);
    
    applyDesiredVelocity(agent, direction*maxSpeed, acceleration);
}

void arrive(GObject* agent, SpaceVect target)
{
	SpaceVect displacement = target - agent->getPos();
	SpaceFloat distance = displacement.length();
	SpaceFloat arrivalTime = distance / agent->getVel().length();
	SpaceFloat accelMag = 2.0 * distance / arrivalTime / arrivalTime;
	SpaceVect direction = displacement.normalizeSafe();

	if (displacement.lengthSq() < 1e-4)
		return;

	applyDesiredVelocity(agent, SpaceVect::zero, accelMag);
}

bool moveToPoint(GObject* agent, SpaceVect target, SpaceFloat arrivalMargin, bool stopForObstacle)
{
	SpaceFloat _accel = agent->getMaxAcceleration();
	SpaceFloat dist2 = (agent->getPos() - target).lengthSq();
	SpaceFloat stoppingDist = getStoppingDistance(agent->getVel().length(), _accel);
	SpaceFloat radius = agent->getRadius();
	SpaceFloat angle = agent->getAngle();

	if (
		stopForObstacle &&
		agent->space->physicsContext->obstacleFeeler(
			agent,
			SpaceVect::ray(stoppingDist+radius, angle),
			radius*2.0
		)
	) {
		applyDesiredVelocity(agent, SpaceVect::zero, _accel);
		return false;
	}
	else if (dist2 < arrivalMargin*arrivalMargin) {
		return true;
	}
	else if (dist2 <= stoppingDist*stoppingDist) {
		arrive(agent, target);
		return false;
	}
	else {
		seek(agent, target, agent->getMaxSpeed(), _accel);
		return false;
	}
}

void flee(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration)
{
	SpaceVect displacement = fleeDirection(agent,target);
    
    applyDesiredVelocity(agent, displacement*maxSpeed, acceleration);
}

SpaceVect fleeDirection(const GObject* agent, SpaceVect target)
{
	SpaceVect displacement = target - agent->getPos();

	if (displacement.lengthSq() < 1e-4)
		return SpaceVect::zero;

	displacement = displacement.normalize();
	displacement = displacement.rotate(float_pi);

	return displacement;
}

void fleeWithObstacleAvoidance(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration)
{
	SpaceVect displacement = fleeDirection(agent, target);
	SpaceFloat distanceMargin = getTurningRadius(agent->getVel().length(), acceleration) + agent->getRadius();

	if (
		agent->space->physicsContext->obstacleDistanceFeeler(
			agent,
			displacement * distanceMargin,
			agent->getRadius()*2.0
		) < distanceMargin
	){
		//Choose an alternate direction to move.

		array<SpaceFloat, 8> feelers = obstacleFeeler8(agent, distanceMargin);

		int directionID = chooseBestDirection(feelers, displacement.toAngle(), distanceMargin);

		if (directionID == -1) {
			applyDesiredVelocity(agent, SpaceVect::zero, acceleration);
		}
		else {
			applyDesiredVelocity(agent, SpaceVect::ray(maxSpeed, directionID* float_pi / 4.0), acceleration);
		}
	}
	else
	{
		flee(agent, target, maxSpeed, acceleration);
	}
}

SpaceFloat getStoppingTime(SpaceFloat speed, SpaceFloat acceleration)
{
	return speed / acceleration;
}

SpaceFloat getStoppingDistance(SpaceFloat speed, SpaceFloat acceleration)
{
	SpaceFloat t = getStoppingTime(speed, acceleration);

	return 0.5 * acceleration * t * t;
}

SpaceFloat getStoppingDistance(GObject* obj)
{
	return getStoppingDistance(obj->getMaxSpeed(), obj->getMaxAcceleration());
}

SpaceFloat getTurningRadius(SpaceFloat speed, SpaceFloat acceleration)
{
	return speed * speed / acceleration;
}

SpaceVect bezier(array<SpaceVect, 3> points, SpaceFloat t)
{
	SpaceFloat u = 1.0 - t;
	return u * u*points[0] + 2.0*u*t*points[1] + t * t*points[2];
}

SpaceVect bezierAcceleration(array<SpaceVect, 3> points)
{
	return 2.0 * (points[2] - 2.0*points[1] - points[0]);
}

float bombScore(GSpace* space, SpaceVect pos, SpaceFloat radius)
{
	float score = 0.0f;

	unordered_set<Agent*> targets = space->physicsContext->radiusQueryByType<Agent>(
		nullptr,
		pos,
		radius,
		enum_bitwise_or(GType, enemy, player),
		PhysicsLayers::all
	);

	for (Agent* target : targets)
	{
		float scale = getExplosionScale(pos, target, radius);
		if (target->getType() == GType::player)
			score += scale;
		else if (target->getType() == GType::enemy)
			score -= scale;
	}
	
	return score;
}

parametric_space_function parametricMoveTranslate(parametric_space_function f, SpaceVect origin, SpaceFloat t_start)
{
	return [f, origin, t_start](SpaceFloat t)->SpaceVect {
		return f(t + t_start) + origin;
	};
}

}//end NS
