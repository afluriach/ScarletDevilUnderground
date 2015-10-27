//
//  GSpace.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include "chipmunk.hpp"

class GSpace
{
public:
    inline GSpace()
    {
        space.setGravity(cp::Vect(0,0));
    }
private:
    cp::Space space;
};

#endif /* GSpace_hpp */
