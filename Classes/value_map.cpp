//
//  value_map.cpp
//  Koumachika
//
//  Created by Toni on 6/27/18.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "GSpace.hpp"
#include "types.h"
#include "util.h"
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

ValueMap getSpawnArgs(const ValueMap& args)
{
	string obj_name = getStringOrDefault(args, "name", "");
	string spawn_name = getStringOrDefault(args, "spawn_name", "");
	string spawn_type = getStringOrDefault(args, "spawn_type", "");
	ValueMap result = args;

	if (spawn_name.empty())
		log("Spawner %s, spawn_name missing!", obj_name.c_str());
	if (spawn_type.empty())
		log("Spawner %s, spawn_type missing!", obj_name.c_str());

	result.insert_or_assign("name", spawn_name);
	result.insert_or_assign("type", spawn_type);

	return result;
}

SpaceVect getObjectPos(const ValueMap& args)
{
	return SpaceVect(getFloat(args, "pos_x"), getFloat(args, "pos_y"));
}

SpaceVect getObjectDimensions(const ValueMap& args)
{
	return SpaceVect(getFloat(args, "dim_x"), getFloat(args, "dim_y"));
}

int getInt(const ValueMap& args, const string& name)
{
	return args.at(name).asInt();
}

int getIntOrDefault(const ValueMap& obj, const string& name, int def)
{
	auto it = obj.find(name);

	if (it != obj.end()) {
		return it->second.asInt();
	}
	else {
		return def;
	}
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

bool getBoolOrDefault(const ValueMap& args, const string& field, bool val)
{
	auto it = args.find(field);

	if (it == args.end()) {
		return val;
	}

	return it->second.asBool();
}

Direction getDirectionOrDefault(const ValueMap& args, Direction d)
{
	auto it = args.find("direction");
	if (it != args.end()) {
		Direction dir = stringToDirection(it->second.asString());
		if (dir != Direction::none)
			return dir;
		else
			return d;
	}
	else {
		return d;
	}
}

ValueMap getMap(const ValueMap& args, const string& field)
{
    auto it = args.find(field);

    if(it != args.end() && it->second.isMap())
        return it->second.asValueMap();
    else
        return ValueMap();
}

ValueVector getVector(const ValueMap& args, const string& field, int start)
{
	ValueVector result;
	int idx = start;

	for (;; ++idx)
	{
		string fieldName = field + boost::lexical_cast<string>(idx);
		auto it = args.find(fieldName);
		if (it == args.end()) break;

		result.push_back(it->second);
	}

	return result;
}

ValueVector getVector(const ValueMap& args, const string& field, int start, int size)
{
	ValueVector result;

	for (int idx = start; result.size() < size; ++idx)
	{
		string fieldName = field + boost::lexical_cast<string>(idx);
		auto it = args.find(fieldName);
		result.push_back(it != args.end() ? it->second : Value());
	}

	return result;
}

gobject_ref getObjRefFromStringField(GSpace* space, const ValueMap& args, const string& fieldName)
{
    if(args.find(fieldName) == args.end()){
        return gobject_ref();
    }
    gobject_ref target = space->getObject(args.at(fieldName).asString());
    
    return target;
}

void convertToUnitSpace(ValueMap& arg, IntVec2 offset)
{
    SpaceVect cornerPos(getFloat(arg, "x"), getFloat(arg, "y"));
    cornerPos *= app::tilesPerPixel;
	cornerPos += SpaceVect(offset.first, offset.second);
    
    SpaceVect dim(getFloat(arg, "width"), getFloat(arg, "height"));
    dim *= app::tilesPerPixel;
    
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

SpaceRect getUnitspaceRectangle(const ValueMap& tileMapObj, IntVec2 offset)
{
    SpaceVect cornerPos(getFloat(tileMapObj, "x"), getFloat(tileMapObj, "y"));
    cornerPos *= app::tilesPerPixel;
	cornerPos += SpaceVect(offset.first, offset.second);
    
    SpaceVect dim(getFloat(tileMapObj, "width"), getFloat(tileMapObj, "height"));
    dim *= app::tilesPerPixel;
    
    return SpaceRect(cornerPos.x, cornerPos.y, dim.x, dim.y);
}
