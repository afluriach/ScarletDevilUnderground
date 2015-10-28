//
//  Block.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef Block_hpp
#define Block_hpp

#include "cocos2d.h"

#include "GObject.hpp"

class Block : public GObject
{
public:
    inline Block(const cocos2d::ValueMap& args) : GObject(args)
    {
        cocos2d::log("Block created.");
    }
};
#endif /* Block_hpp */
