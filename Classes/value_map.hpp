//
//  value_map.hpp
//  Koumachika
//
//  Created by Toni on 6/27/18.
//
//

#ifndef value_map_hpp
#define value_map_hpp

#include "object_ref.hpp"

void printValueMap(const ValueMap& obj);
bool validateStringArg(const ValueMap& args, string name);
bool validateMapArg(const ValueMap& args, string name);

float getFloat(const ValueMap& obj, const string& name);
float getFloatOrDefault(const ValueMap& obj, const string& name, float def);

string getStringOrDefault(const ValueMap& args, const string& field, const string& _default);

ValueMap getMap(const ValueMap& args, const string& field);

gobject_ref getObjRefFromStringField(const ValueMap& args, const string& fieldName);

void convertToUnitSpace(ValueMap& arg);
cocos2d::CCRect getUnitspaceRectangle(const ValueMap& tileMapObj);

#endif /* value_map_hpp */