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

void printValueMap(const ValueMap& obj);
bool validateStringArg(const ValueMap& args, string name);
bool validateMapArg(const ValueMap& args, string name);

ValueMap getSpawnArgs(const ValueMap& args);

SpaceVect getObjectPos(const ValueMap& args);
SpaceVect getObjectDimensions(const ValueMap& args);

int getInt(const ValueMap& obj, const string& name);
int getIntOrDefault(const ValueMap& obj, const string& name, int def);

float getFloat(const ValueMap& obj, const string& name);
float getFloatOrDefault(const ValueMap& obj, const string& name, float def);

string getStringOrDefault(const ValueMap& args, const string& field, const string& _default);

bool getBoolOrDefault(const ValueMap& args, const string& field, bool val);

Direction getDirectionOrDefault(const ValueMap& args, Direction d);

ValueMap getMap(const ValueMap& args, const string& field);
ValueVector getVector(const ValueMap& args, const string& field, int start);
ValueVector getVector(const ValueMap& args, const string& field, int start, int size);

gobject_ref getObjRefFromStringField(GSpace* space, const ValueMap& args, const string& fieldName);

void convertToUnitSpace(ValueMap& arg, IntVec2 offset);
SpaceRect getUnitspaceRectangle(const ValueMap& tileMapObj, IntVec2 offset);

inline vector<string> getStringVector(
	const ValueMap& obj,
	string name,
	int startIdx = 1
) {
	vector<string> result;
	ValueVector fields = getVector(obj, name, startIdx);

	for (Value v : fields) {
		result.push_back(v.asString());
	}

	return result;
}

template<typename T>
inline vector<T> getObjectVector(
	const ValueMap& obj,
	function<T(string)> converter,
	string name,
	int startIdx = 1
){
	vector<T> result;
	ValueVector fields = getVector(obj, name, startIdx);

	for (Value v : fields){
		result.push_back(converter(v.asString()));
	}
	
	return result;
}

template<typename T>
inline vector<T> getObjectVector(
	const ValueMap& obj,
	function<T(string)> converter,
	string name,
	int startIdx,
	int size,
	T _default
){
	vector<T> result;
	ValueVector fields = getVector(obj, name, startIdx, size);

	for (Value v : fields){
		result.push_back(v.isNull() ? _default : converter(v.asString()));
	}

	return result;
}

#endif /* value_map_hpp */
