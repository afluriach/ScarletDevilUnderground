//
//  StateMapping.cpp
//  Koumachika
//
//  Created by Toni on 3/15/18.
//
//

#include "Prefix.h"

#include "AI.hpp"

using namespace ai;

template <typename T>
static Function::AdapterType consAdapter()
{
    return [](const ValueMap& args) -> shared_ptr<Function> { return make_shared<T>(args); };
}

#define entry(name,cls) (name, consAdapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

const unordered_map<string, Function::AdapterType> Function::adapters = boost::assign::map_list_of
    entry_same(Detect)
    entry_same(Seek)
    entry_same(MaintainDistance)
    entry_same(Flee)
    entry_same(DetectAndSeekPlayer)
    entry_same(IdleWait)
    entry_same(MoveToPoint)
	entry_same(FollowPath)
    entry_same(Wander)

    entry_same(Cast)
    entry_same(IllusionDash)
;
