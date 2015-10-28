//
//  GObject.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Block.hpp"
#include "GObject.hpp"
#include "util.h"

USING_NS_CC;

GObject::GObject(const ValueMap& obj)
{
    log("GObject instantiated.");
    printValueMap(obj);
}

const std::map<std::string,GObject::AdapterType> GObject::adapters = map_list_of("Block", GObject::consAdapter<Block>());

GObject* GObject::constructByType(const std::string& type, const cocos2d::ValueMap& args )
{
    auto it = adapters.find(type);
    
    if(it != adapters.end()){
        AdapterType adapter =  it->second;
        return adapter(args);
    }
    else return nullptr;
}