//
//  AIUtil.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef AIUtil_hpp
#define AIUtil_hpp

#include "AI.hpp"

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject* obj, SpaceVect desired, SpaceFloat maxForce);
SpaceVect compute_seek(Agent* agent, SpaceVect target);
void seek(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
void arrive(GObject* agent, SpaceVect target);
SpaceVect fleeDirection(const GObject* agent, SpaceVect target);
void flee(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
void fleeWithObstacleAvoidance(GObject* agent, SpaceVect target, SpaceFloat maxSpeed, SpaceFloat acceleration);
bool moveToPoint(GObject* agent, SpaceVect target, SpaceFloat arrivalMargin, bool stopForObstacle);

bool isFacingTarget(const GObject* agent, const GObject* target);
bool isFacingTargetsBack(const GObject* agent, const GObject* target);
bool isLineOfSight(const GObject* agent, const GObject* target);
bool isObstacle(Agent* agent, SpaceVect target);

array<SpaceFloat, 8> wallFeeler8(const GObject* agent, SpaceFloat distance);
array<SpaceFloat, 4> obstacleFeelerQuad(const GObject* agent, SpaceFloat distance);
array<SpaceFloat, 8> obstacleFeeler8(const GObject* agent, SpaceFloat distance);
int chooseBestDirection(const array<SpaceFloat, 8>& feelers, SpaceFloat desired_angle, SpaceFloat min_distance);

SpaceVect directionToTarget(const GObject* agent, SpaceVect target);
SpaceVect displacementToTarget(const GObject* agent, SpaceVect target);
SpaceFloat distanceToTarget(const GObject* agent, const GObject* target);
SpaceFloat distanceToTarget(const GObject* agent, SpaceVect target);
SpaceFloat distanceToTarget(SpaceVect pos, SpaceVect target);
SpaceFloat viewAngleToTarget(const GObject* agent, const GObject* target);
SpaceVect projectileEvasion(const GObject* bullet, const GObject* agent);
bool isInFieldOfView(GObject* agent, SpaceVect target, SpaceFloat fovAngleScalarProduct);

SpaceFloat getStoppingTime(SpaceFloat speed, SpaceFloat acceleration);
SpaceFloat getStoppingDistance(SpaceFloat speed, SpaceFloat accceleration);
SpaceFloat getStoppingDistance(GObject* obj);
SpaceFloat getTurningRadius(SpaceFloat speed, SpaceFloat acceleration);

SpaceVect bezier(array<SpaceVect, 3> points, SpaceFloat t);
SpaceVect bezierAcceleration(array<SpaceVect, 3> points);

float bombScore(GSpace* space, SpaceVect pos, SpaceFloat radius);

} //end NS

#endif /* AIUtil_hpp */
