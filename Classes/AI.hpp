//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

namespace ai{

//Low-level movement functions.

//Not strictly an AI function since it's used to control the player.
void applyDesiredVelocity(GObject& obj, const SpaceVect& desired, float maxForce);
void seek(GObject& agent, GObject& target, float maxSpeed, float acceleration);
Vec2 directionToTarget(GObject& agent, GObject& target);

} //end NS

#endif /* AI_hpp */
