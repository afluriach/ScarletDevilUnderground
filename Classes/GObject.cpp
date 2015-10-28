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

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
static GObject::AdapterType consAdapter()
{
    return [](const cocos2d::ValueMap& args) -> GObject* { return new T(args); };
}

const std::map<std::string,GObject::AdapterType> GObject::adapters = map_list_of("Block", consAdapter<Block>());

GObject::GObject(const ValueMap& obj) : name(obj.at("name").asString() )
{
    log("GObject %s instantiated.", name.c_str());
    printValueMap(obj);
    
    cp::Vect cornerPos(getFloat(obj, "x"), getFloat(obj, "y"));
    cornerPos *= App::tilesPerPixel;
    
    dim = cp::Vect(getFloat(obj, "width"), getFloat(obj, "height"));
    dim *= App::tilesPerPixel;
    
    initialCenter = cp::Vect(cornerPos);
    initialCenter += (dim*0.5);
}


GObject* GObject::constructByType(const std::string& type, const cocos2d::ValueMap& args )
{
    auto it = adapters.find(type);
    
    if(it != adapters.end()){
        AdapterType adapter =  it->second;
        return adapter(args);
    }
    else return nullptr;
}

std::shared_ptr<cp::Body> GObject::initRectangleBody(cp::Space& space)
{
    body = GSpace::createRectangleBody(space, initialCenter, dim, mass, this);
    return body;
}