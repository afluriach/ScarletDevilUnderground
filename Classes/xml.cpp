//
//  xml.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "Attributes.hpp"
#include "Bullet.hpp"
#include "FileIO.hpp"
#include "Graphics.h"
#include "graphics_types.h"

namespace app {

unordered_map<string, AttributeMap> attributes;
unordered_map<string, shared_ptr<bullet_properties>> bullets;
unordered_map<string, floorsegment_properties> floors;
unordered_map<string, shared_ptr<LightArea>> lights;
unordered_map<string, sprite_properties> sprites;

void loadAttributes()
{
	loadObjects<AttributeMap>("objects/attributes.xml", app::attributes);
}

void loadBullets()
{
	loadObjects<shared_ptr<bullet_properties>>("objects/bullets.xml", app::bullets);
}

void loadFloors()
{
	loadObjects<floorsegment_properties>("objects/floors.xml", app::floors);
}

void loadLights()
{
	loadObjects<shared_ptr<LightArea>>("objects/lights.xml", app::lights);
}

void loadSprites()
{
	loadObjects<sprite_properties>("objects/sprites.xml", app::sprites);
}

shared_ptr<bullet_properties> getBullet(const string& name)
{
	return getOrDefault(bullets, name);
}

shared_ptr<LightArea> getLight(const string& name)
{
	return getOrDefault(lights, name);
}

sprite_properties getSprite(const string& name)
{
	return getOrDefault(sprites, name);
}

AttributeMap getAttributes(const string& name)
{
	return getOrDefault(attributes, name);
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
			*result = boost::lexical_cast<T, string>(attr);
			return true;
		}
		catch (boost::bad_lexical_cast ex) {
			log("Unable to parse XML attribute %s", name);
		}
	}
	return false;
}

bool parseObject(tinyxml2::XMLElement* elem, AttributeMap* result)
{
	AttributeMap _result;

	for (
		tinyxml2::XMLElement* crnt = elem->FirstChildElement();
		crnt != nullptr;
		crnt = crnt->NextSiblingElement()
	){
		const char* attrName = crnt->Name();
		Attribute crntAttr = AttributeSystem::getAttribute(attrName);
		float val;

		if (crntAttr != Attribute::end && getNumericAttr(crnt, "val", &val)) {
			_result.insert_or_assign(crntAttr, val);
		}
		else {
			log("parseObject: unknown attribute %s", attrName);
			return false;
		}
	}

	*result = _result;
	return true;
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

bool parseObject(tinyxml2::XMLElement* elem, IntVec2* result)
{
	string val;
	if (getStringAttr(elem, "size", &val)) {
		*result = toIntVector(val);
		return false;
	}
	return false;
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

bool parseObject(tinyxml2::XMLElement* elem, sprite_properties* result)
{
	string filename;
	IntVec2 size = make_pair(1, 1);
	int dpi = 128;
	float referenceSize = 0.0f;
	Color4F _color = Color4F::WHITE;

	if (!getStringAttr(elem, "file", &filename)) {
		filename = elem->Name();
	}
	if (!FileUtils::getInstance()->isFileExist("sprites/" + filename + ".png")) {
		log("Sprite %s not found!", filename);
		return false;
	}

	parseObject(elem, &size);
	parseObject(elem, &_color);
	getNumericAttr(elem, "dpi", &dpi);
	getNumericAttr(elem, "ref-size", &referenceSize);

	*result = sprite_properties{
		filename,
		size,
		dpi,
		referenceSize,
		toColor3B(_color)
	};
	return true;
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

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<bullet_properties>* result)
{
	SpaceFloat speed;
	SpaceFloat radius;

	DamageInfo damage;
	damage.element = Attribute::end;
	damage.type = DamageType::bullet;

	string sprite;
	string lightSource;

	int hitCount = 1;
	int ricochetCount = 0;
	bool directionalLaunch = true;

	getNumericAttr(elem, "speed", &speed);
	getNumericAttr(elem, "radius", &radius);

	getNumericAttr(elem, "damage", &damage.mag);

	const char* elemental = elem->Attribute("element");
	if (elemental) {
		damage.element = AttributeSystem::getAttribute(elemental);
	}

	getStringAttr(elem, "sprite", &sprite);
	getStringAttr(elem, "lightSource", &lightSource);
	getNumericAttr(elem, "hitCount", &hitCount);
	getNumericAttr(elem, "ricochet", &ricochetCount);
	getNumericAttr(elem, "directionalLaunch", &directionalLaunch);

	*result = make_shared<bullet_properties>(bullet_properties{
		0.1,
		speed,
		radius,
		damage,
		sprite,
		lightSource,
		to_char(hitCount),
		to_char(ricochetCount),
		directionalLaunch
	});
	return true;
}

}
