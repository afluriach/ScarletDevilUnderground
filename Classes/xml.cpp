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
#include "FirePatternImpl.hpp"
#include "Graphics.h"
#include "graphics_types.h"

namespace app {

unordered_map<string, AttributeMap> attributes;
unordered_map<string, shared_ptr<bullet_properties>> bullets;
unordered_map<string, shared_ptr<enemy_properties>> enemies;
unordered_map<string, shared_ptr<firepattern_properties>> firePatterns;
unordered_map<string, floorsegment_properties> floors;
unordered_map<string, shared_ptr<LightArea>> lights;
unordered_map<string, sprite_properties> sprites;

GObject::AdapterType enemyAdapter(shared_ptr<enemy_properties> props)
{
	return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		return new EnemyImpl(space, id, args, props);
	};
}

void loadAttributes()
{
	loadObjects<AttributeMap>("objects/attributes.xml", app::attributes);
}

void loadBullets()
{
	loadObjects<shared_ptr<bullet_properties>>("objects/bullets.xml", app::bullets);
}

void loadEnemies()
{
	loadObjects<shared_ptr<enemy_properties>>("objects/enemies.xml", app::enemies);

	for (auto entry : enemies)
	{
		GObject::namedObjectTypes.insert_or_assign(entry.first, enemyAdapter(entry.second));
	}
}

void loadFirePatterns()
{
	loadObjects<shared_ptr<firepattern_properties>>("objects/fire-patterns.xml", app::firePatterns);
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

shared_ptr<enemy_properties> getEnemy(const string& name)
{
	return getOrDefault(enemies, name);
}

shared_ptr<firepattern_properties> getFirePattern(const string& name)
{
	return getOrDefault(firePatterns, name);
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

bool getColorAttr(tinyxml2::XMLElement* elem, const string& name, Color4F* result)
{
	if (auto attr = elem->Attribute(name.c_str())) {
		*result = toColor(string(attr));
		return true;
	}
	return false;
}

bool getVector(tinyxml2::XMLElement* elem, const string& name, SpaceVect* result)
{
	const char* attr = elem->Attribute(name.c_str());

	if (attr) {
		try {
			vector<string> tokens = splitString(attr, ",");
			if (tokens.size() == 2) {
				SpaceVect _result;
				_result.x = boost::lexical_cast<double>(tokens.at(0));
				_result.y = boost::lexical_cast<double>(tokens.at(1));
				*result = _result;
				return true;
			}
		}
		catch (boost::bad_lexical_cast ex) {
			log("Unable to parse XML attribute %s", name);
		}
	}
	return false;
}

DamageInfo getDamageInfo(tinyxml2::XMLElement* elem, DamageType type)
{
	DamageInfo result = { 0.0f, Attribute::end, type };

	getNumericAttr(elem, "damage", &result.mag);

	const char* elemental = elem->Attribute("element");
	if (elemental) {
		result.element = AttributeSystem::getAttribute(elemental);
	}

	return result;
}

bool autoName(const string& name, string& field)
{
	bool result = field == "auto";
	if (result) {
		field = name;
	}
	return result;
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

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<enemy_properties>* result)
{
	enemy_properties props;
	props.radius = Agent::defaultSize;
	const char* collectibleAttr;
	const char* lightAttr;

	getStringAttr(elem, "name", &props.name);
	getStringAttr(elem, "sprite", &props.sprite);
	getStringAttr(elem, "attributes", &props.attributes);
	getStringAttr(elem, "ai_package", &props.ai_package);

	autoName(elem->Name(), props.sprite);
	autoName(elem->Name(), props.attributes);
	autoName(elem->Name(), props.ai_package);

	getNumericAttr(elem, "radius", &props.radius);
	getNumericAttr(elem, "mass", &props.mass);
	getNumericAttr(elem, "viewAngle", &props.viewAngle);
	getNumericAttr(elem, "viewRange", &props.viewRange);

	props.touchEffect = getDamageInfo(elem, DamageType::touch);

	collectibleAttr = elem->Attribute("collectible");

	if (collectibleAttr) {
		props.collectible = Collectible::getCollectibleID(collectibleAttr);
	}

	lightAttr = elem->Attribute("light");

	if (lightAttr) {
		props.lightSource = getLight(lightAttr);
	}

	getNumericAttr(elem, "detectEssence", &props.detectEssence);
	getNumericAttr(elem, "isFlying", &props.isFlying);

	*result = make_shared<enemy_properties>(props);
	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<firepattern_properties>* _output)
{
	SpaceFloat distance = FirePattern::defaultLaunchDistance;
	SpaceFloat interval = 0.0;
	SpaceFloat burstInterval = 0.0;
	int burstCount = 1;

	SpaceFloat sideAngle = 0.0;
	int bulletCount = 1;

	float staminaCost = 0.0f;

	string iconRes;
	string bulletName;
	string type;
	bool match = false;
	if (!getStringAttr(elem, "type", &type)) {
		log("fire pattern %s does not have a type.", elem->Name());
		return false;
	}

	getNumericAttr(elem, "distance", &distance);
	getNumericAttr(elem, "interval", &interval);
	getNumericAttr(elem, "burstInterval", &burstInterval);
	getNumericAttr(elem, "burstCount", &burstCount);

	getNumericAttr(elem, "sideAngle", &sideAngle);
	getNumericAttr(elem, "bulletCount", &bulletCount);

	getNumericAttr(elem, "cost", &staminaCost);

	getStringAttr(elem, "bullet", &bulletName);
	getStringAttr(elem, "icon", &iconRes);

	if (boost::iequals(type, "single")) {
		*_output = firepattern_properties::makeSingle(
			app::getBullet(bulletName),
			interval,
			distance,
			burstInterval,
			burstCount
		);
		match = true;
	}
	else if (boost::iequals(type, "spread")){
		*_output = firepattern_properties::makeSpread(
			app::getBullet(bulletName),
			interval,
			distance,
			burstInterval,
			burstCount,
			toRads(sideAngle),
			bulletCount
		);
		match = true;
	}
	else if (boost::iequals(type, "radius")) {
		*_output = firepattern_properties::makeRadius(
			app::getBullet(bulletName),
			interval,
			distance,
			bulletCount
		);
		match = true;
	}

	if (match) {
		(*_output)->staminaCost = staminaCost;
		(*_output)->icon = iconRes;
	}

	return match;
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

bool parseObject(tinyxml2::XMLElement* elem, sprite_properties* result)
{
	string filename;
	IntVec2 size = make_pair(1, 1);
	int dpi = 128;
	float duration = 0.0f;
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
	getColorAttr(elem, "color", &_color);
	getNumericAttr(elem, "dpi", &dpi);
	getNumericAttr(elem, "duration", &duration);
	getNumericAttr(elem, "ref-size", &referenceSize);

	*result = sprite_properties{
		filename,
		size,
		dpi,
		referenceSize,
		duration,
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
			getColorAttr(elem, "color", &_result->color);

			(*result) = _result;
			return true;
		}
		else if (type == "sprite")
		{
			auto _result = make_shared<SpriteLightArea>();
			string spriteName;

			getColorAttr(elem, "color", &_result->color);
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
	SpaceFloat speed = 0.0;
	SpaceVect dimensions;
	SpaceFloat knockback = 0.0;

	DamageInfo damage;

	string sprite;
	string lightSource;

	int hitCount = 1;
	int ricochetCount = 0;
	bool directionalLaunch = true;
	bool ignoreObstacles = false;
	bool deflectBullets = false;

	getNumericAttr(elem, "speed", &speed);
	if (!getVector(elem, "dimensions", &dimensions)) {
		getNumericAttr(elem, "radius", &dimensions.x);
	}

	damage = getDamageInfo(elem, DamageType::bullet);

	getStringAttr(elem, "sprite", &sprite);
	getStringAttr(elem, "lightSource", &lightSource);
	getNumericAttr(elem, "hitCount", &hitCount);
	getNumericAttr(elem, "ricochet", &ricochetCount);
	getNumericAttr(elem, "directionalLaunch", &directionalLaunch);
	getNumericAttr(elem, "ignoreObstacles", &ignoreObstacles);
	getNumericAttr(elem, "deflectBullets", &deflectBullets);

	*result = make_shared<bullet_properties>(bullet_properties{
		0.1,
		speed,
		dimensions,
		knockback,
		damage,
		sprite,
		lightSource,
		to_char(hitCount),
		to_char(ricochetCount),
		directionalLaunch,
		ignoreObstacles,
		deflectBullets
	});
	return true;
}

}
