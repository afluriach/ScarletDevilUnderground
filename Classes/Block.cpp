//
//  Block.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Block.hpp"

std::shared_ptr<cp::Body> Block::initializeBody(cp::Space& space)
{
    return GObject::initRectangleBody(space);
}