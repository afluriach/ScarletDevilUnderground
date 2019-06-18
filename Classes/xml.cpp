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

bool parseObject(tinyxml2::XMLElement* elem, floorsegment_properties* result)
{
	string sfx;
	double traction = 1.0;

	if (auto attr = elem->Attribute("sfx"))
		sfx = string(attr);

	if (auto attr = elem->Attribute("traction"))
		traction = elem->DoubleAttribute("traction");

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

			if (auto attr = elem->Attribute("radius"))
				_result->radius = elem->DoubleAttribute("radius");

			if (auto attr = elem->Attribute("flood"))
				_result->flood = elem->DoubleAttribute("flood");

			parseObject(elem, &_result->color);

			(*result) = _result;
			return true;
		}
		else if (type == "sprite")
		{
			auto _result = make_shared<SpriteLightArea>();

			parseObject(elem, &_result->color);

			if (auto attr = elem->Attribute("sprite"))
				_result->texName = string("sprites/") + string(attr) + string(".png");

			if (auto attr = elem->Attribute("scale"))
				_result->scale = elem->DoubleAttribute("scale");

			(*result) = _result;
			return true;
		}
	}

	return false;
}

}
