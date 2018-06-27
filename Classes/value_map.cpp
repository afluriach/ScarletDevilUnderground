//
//  value_map.cpp
//  Koumachika
//
//  Created by Toni on 6/27/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "types.h"
#include "value_map.hpp"

void printValueMap(const ValueMap& obj)
{
    for(auto it = obj.begin(); it != obj.end(); ++it)
    {
        log(
            "%s: %s.",
            it->first.c_str(),
            it->second.asString().c_str()
        );
    }
}

bool validateStringArg(const ValueMap& args, string name)
{
    auto it = args.find(name);
    
    return ( it != args.end() && args.at(name).isString() && args.at(name).asString() != "" );
}

bool validateMapArg(const ValueMap& args, string name)
{
    auto it = args.find(name);
    
    return ( it != args.end() && args.at(name).isMap() );
}

float getFloat(const ValueMap& args, const string& name)
{
    return args.at(name).asFloat();
}

float getFloatOrDefault(const ValueMap& args, const string& name, float def)
{
    if(args.find(name) != args.end())
        return getFloat(args,name);
    else return def;
}

string getStringOrDefault(const ValueMap& args, const string& field, const string& _default)
{
    auto it = args.find(field);
    
    if(it == args.end() || !it->second.isString()){
        return _default;
    }
    return it->second.asString();
}

ValueMap getMap(const ValueMap& args, const string& field)
{
    auto it = args.find(field);

    if(it != args.end() && it->second.isMap())
        return it->second.asValueMap();
    else
        return ValueMap();
}

gobject_ref getObjRefFromStringField(const ValueMap& args, const string& fieldName)
{
    if(args.find(fieldName) == args.end()){
        return gobject_ref();
    }
    gobject_ref target = app->space->getObject(args.at(fieldName).asString());
    
    return target;
}

void convertToUnitSpace(ValueMap& arg)
{
    SpaceVect cornerPos(getFloat(arg, "x"), getFloat(arg, "y"));
    cornerPos *= App::tilesPerPixel;
    
    SpaceVect dim(getFloat(arg, "width"), getFloat(arg, "height"));
    dim *= App::tilesPerPixel;
    
    SpaceVect center = SpaceVect(cornerPos);
    center += (dim*0.5);
    
    arg.erase("x");
    arg.erase("y");

    arg.erase("width");
    arg.erase("height");
    
    arg["pos_x"] = Value(center.x);
    arg["pos_y"] = Value(center.y);
    
    arg["dim_x"] = Value(dim.x);
    arg["dim_y"] = Value(dim.y);
}

cocos2d::CCRect getUnitspaceRectangle(const ValueMap& tileMapObj)
{
    SpaceVect cornerPos(getFloat(tileMapObj, "x"), getFloat(tileMapObj, "y"));
    cornerPos *= App::tilesPerPixel;
    
    SpaceVect dim(getFloat(tileMapObj, "width"), getFloat(tileMapObj, "height"));
    dim *= App::tilesPerPixel;
    
    return cocos2d::CCRect(cornerPos.x, cornerPos.y, dim.x, dim.y);
}
