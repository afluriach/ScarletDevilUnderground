//
//  xml.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "FileIO.hpp"
#include "Graphics.h"
#include "graphics_types.h"

namespace app {

unordered_map<string, floorsegment_properties> floors;
unordered_map<string, shared_ptr<LightArea>> lights;

void loadFloors()
{
	loadObjects<floorsegment_properties>("objects/floors.xml", app::floors);
}

void loadLights()
{
	loadObjects<shared_ptr<LightArea>>("objects/lights.xml", app::lights);
}

shared_ptr<LightArea> getLight(const string& name)
{
	return getOrDefault(lights, name, shared_ptr<LightArea>(nullptr));
}

bool getStringAttr(tinyxml2::XMLElement* elem, const string& name, string* result)
{
	const char* attr = elem->Attribute(name.c_str());

	if (attr)
		*result = string(attr);

	return attr;
}

template<typename T>
bool getNumericAttr(tinyxml2::XMLElement* elem, const string& name, T* result)
{
	const char* attr = elem->Attribute(name.c_str());

	if (attr) {
		try {
			*result = boost::lexical_cast<T>(attr);
			return true;
		}
		catch (boost::bad_lexical_cast ex) {}
	}
	return false;
}

bool parseObject(tinyxml2::XMLElement* elem, floorsegment_properties* result)
{
	string sfx;
	double traction = 1.0;

	getStringAttr(elem, "sfx", &sfx);
	getNumericAttr(elem, "traction", &traction);

	*result = floorsegment_properties{ sfx, traction };
	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, Color4F* result)
{
	if (auto attr = elem->Attribute("color")) {
		*result = toColor4F(string(attr));
		return true;
	}
	else if (auto attr = elem->Attribute("color-3b")) {
		*result = toColor4F(toColor3B(string(attr)));
		return true;
	}
	else if (auto attr = elem->Attribute("color-hsv")) {
		*result = hsva4F(string(attr));
		return true;
	}
	else {
		return false;
	}
}

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<LightArea>* result)
{
	if (auto attr = elem->Attribute("type"))
	{
		string type = string(attr);

		if (type == "circle")
		{
			auto _result = make_shared<CircleLightArea>();

			getNumericAttr(elem, "radius", &_result->radius);
			getNumericAttr(elem, "flood", &_result->flood);
			parseObject(elem, &_result->color);

			(*result) = _result;
			return true;
		}
		else if (type == "sprite")
		{
			auto _result = make_shared<SpriteLightArea>();
			string spriteName;

			parseObject(elem, &_result->color);
			getNumericAttr(elem, "scale", &_result->scale);

			if (getStringAttr(elem, "sprite", &spriteName)) {
				_result->texName = string("sprites/") + spriteName + string(".png");
			}

			(*result) = _result;
			return true;
		}
	}

	return false;
}

}
