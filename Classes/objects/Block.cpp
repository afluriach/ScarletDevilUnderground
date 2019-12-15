//
//  Block.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "Block.hpp"
#include "value_map.hpp"

Block::Block(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), MapRectPhys(GType::environment, eyeLevelHeightLayers, 40.0))
{
}
