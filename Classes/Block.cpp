//
//  Block.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "Block.hpp"
#include "macros.h"

Block::Block(const ValueMap& args) :
GObject(args),
isStatic(args.find("static") != args.end())
{
    auto it = args.find("letter");
    if(it != args.end())
        letter = it->second.asString();
//        else
//            log("%s: letter undefined", name.c_str());
}

PhysicsLayers Block::getLayers() const{
    return enum_bitwise_or(PhysicsLayers,ground,eyeLevel);
}
