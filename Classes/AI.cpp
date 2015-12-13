//
//  AI.cpp
//  FlansBasement
//
//  Created by Toni on 12/13/15.
//
//

#include "Prefix.h"
#include "AI.hpp"

namespace ai{

//It seems to better to base this on acceleration than force. In some cases mass may
//just be a default value, in which case acceleration makes more sense.
void applyDesiredVelocity(GObject& obj, const SpaceVect& desired, float acceleration)
{
    //the desired velocity change
    SpaceVect vv = desired - obj.getVel();
    //the scalar amount of velocity change in one frame
    float dv = acceleration * App::secondsPerFrame;

    //Default case, apply maximum acceleration
    if(square(dv) < vv.lengthSq()){
        float f = obj.body->getMass() * acceleration;
        SpaceVect ff = f * vv.normalizeSafe();
        obj.applyForceForSingleFrame(ff);
    }
    else{
        obj.setVel(desired);
    }
}

}//end NS