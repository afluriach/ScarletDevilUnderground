//
//  value_map.hpp
//  Koumachika
//
//  Created by Toni on 6/27/18.
//
//

#ifndef value_map_hpp
#define value_map_hpp

class GSpace;

#include "object_ref.hpp"

void printValueMap(const ValueMap& obj);
bool validateStringArg(const ValueMap& args, string name);
bool validateMapArg(const ValueMap& args, string name);

ValueMap getSpawnArgs(const ValueMap& args);

int getInt(const ValueMap& obj, const string& name);

float getFloat(const ValueMap& obj, const string& name);
float getFloatOrDefault(const ValueMap& obj, const string& name, float def);

string getStringOrDefault(const ValueMap& args, const string& field, const string& _default);

bool getBoolOrDefault(const ValueMap& args, const string& field, bool val);

ValueMap getMap(const ValueMap& args, const string& field);

gobject_ref getObjRefFromStringField(GSpace* space, const ValueMap& args, const string& fieldName);

void convertToUnitSpace(ValueMap& arg, IntVec2 offset);
cocos2d::CCRect getUnitspaceRectangle(const ValueMap& tileMapObj, IntVec2 offset);

#endif /* value_map_hpp */
