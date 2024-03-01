//
//  xml.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "FileIO.hpp"
#include "FirePatternImpl.hpp"
#include "Graphics.h"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "PlayScene.hpp"
#include "SpellDescriptor.hpp"
#include "xml_impl.hpp"

namespace app {

unordered_map<string, shared_ptr<area_properties>> areas;
unordered_map<string, MagicEffectDescriptor*> effects;
unordered_map<string, local_shared_ptr<firepattern_properties>> firePatterns;
unordered_map<string, local_shared_ptr<floorsegment_properties>> floors;
unordered_map<string, shared_ptr<LightArea>> lights;
unordered_map<string, local_shared_ptr<object_properties>> objects;
unordered_map<string, SpellDesc*> spells;
unordered_map<string, shared_ptr<sprite_properties>> sprites;

const vector<string> xmlErrors = boost::assign::list_of
    ("XML_NO_ERROR")

    ("XML_NO_ATTRIBUTE")
    ("XML_WRONG_ATTRIBUTE_TYPE")

    ("XML_ERROR_FILE_NOT_FOUND")
    ("XML_ERROR_FILE_COULD_NOT_BE_OPENED")
    ("XML_ERROR_FILE_READ_ERROR")
    ("XML_ERROR_ELEMENT_MISMATCH")
    ("XML_ERROR_PARSING_ELEMENT")
    ("XML_ERROR_PARSING_ATTRIBUTE")
    ("XML_ERROR_IDENTIFYING_TAG")
    ("XML_ERROR_PARSING_TEXT")
    ("XML_ERROR_PARSING_CDATA")
    ("XML_ERROR_PARSING_COMMENT")
    ("XML_ERROR_PARSING_DECLARATION")
    ("XML_ERROR_PARSING_UNKNOWN")
    ("XML_ERROR_EMPTY_DOCUMENT")
    ("XML_ERROR_MISMATCHED_ELEMENT")
    ("XML_ERROR_PARSING")

    ("XML_CAN_NOT_CONVERT_TEXT")
    ("XML_NO_TEXT_NODE")
;

void loadAreas()
{
	loadObjects<shared_ptr<area_properties>>("objects/areas.xml", app::areas);
}

void loadBombs()
{
	loadObjectsShared<bomb_properties>("objects/bombs.xml");
}

void loadBullets()
{
	loadObjectsShared<bullet_properties>("objects/bullets.xml");

	auto f = GSpace::scriptVM->getFunction("loadBullets");
	if (f) f();
}

void loadEffectAreas()
{
	loadObjectsShared<effectarea_properties>("objects/effect-areas.xml");
}

void loadEffects()
{
	loadObjects<MagicEffectDescriptor*>("objects/magic-effects.xml", app::effects);
}

void loadSpells()
{
	loadObjects<SpellDesc*>("objects/spells.xml", app::spells);
}

void loadEnemies()
{
	loadObjectsShared<enemy_properties>("objects/enemies.xml");
}

void loadEnvironmentObjects()
{
	loadObjectsShared<environment_object_properties>("objects/objects.xml");
}

void loadFirePatterns()
{
	loadObjects<local_shared_ptr<firepattern_properties>>("objects/fire-patterns.xml", app::firePatterns);
}

void loadFloors()
{
	loadObjectsShared<floorsegment_properties>("objects/floors.xml");
}

void loadItems()
{
	loadObjectsShared<item_properties>("objects/items.xml");
}

void loadLights()
{
	loadObjects<shared_ptr<LightArea>>("objects/lights.xml", app::lights);
}

void loadNPCs()
{
	loadObjectsShared<npc_properties>("objects/npc.xml");
}

void loadPlayers()
{
	loadObjectsShared<agent_properties>("objects/players.xml");
}

void loadWalls()
{
	loadObjectsShared<wall_properties>("objects/walls.xml");
}

void loadSprites()
{
	loadObjects<shared_ptr<sprite_properties>>("objects/sprites.xml", app::sprites);
}

shared_ptr<area_properties> getArea(const string& name)
{
	return getOrDefault(areas, name);
}

local_shared_ptr<bomb_properties> getBomb(const string& name)
{
	return getObjectProperties<bomb_properties>(name);
}

local_shared_ptr<bullet_properties> getBullet(const string& name)
{
	return getObjectProperties<bullet_properties>(name);
}

const MagicEffectDescriptor* getEffect(const string& name)
{
	return getOrDefault(effects, name);
}

local_shared_ptr<firepattern_properties> getFirePattern(const string& name)
{
	return getOrDefault(firePatterns, name);
}

local_shared_ptr<floorsegment_properties> getFloor(const string& name)
{
    return getObjectProperties<floorsegment_properties>(name);
}

local_shared_ptr<item_properties> getItem(const string& name)
{
	return getObjectProperties<item_properties>(name);
}

shared_ptr<LightArea> getLight(const string& name)
{
	return getOrDefault(lights, name);
}

local_shared_ptr<object_properties> getObjectProps(const string& name)
{
    return getOrDefault(objects, name);
}

local_shared_ptr<agent_properties> getPlayer(const string& name)
{
	return getObjectProperties<agent_properties>(name);
}

local_shared_ptr<wall_properties> getWall(const string& name)
{
	return getObjectProperties<wall_properties>(name);
}

const SpellDesc* getSpell(const string& name)
{
	return getOrDefault(spells, name);
}

shared_ptr<sprite_properties> getSprite(const string& name)
{
	return getOrDefault(sprites, name);
}

local_shared_ptr<bullet_properties> addBullet(const string& name, const string& base)
{
	local_shared_ptr<bullet_properties> result;
	
	if (name.empty()) return nullptr;
    
	if (base.size() > 0) {
		result = copyBaseObjectShared<bullet_properties>(base);

		if (!result) {
			log2("addBullet(%s,%s): base not found", name, base);
			return nullptr;
		}
	}
	else {
		result = make_local_shared<bullet_properties>();
		result->clsName = name;
	}

	objects.insert_or_assign(name, result);
	return result;
}

//get Attribute attribute [sic]
bool getAttributeAttr(tinyxml2::XMLElement* elem, const string& name, Attribute* result)
{
	const char* attr = elem->Attribute(name.c_str());
	bool success = false;

	if (attr) {
		Attribute crntAttr = AttributeSystem::getAttribute(attr);
		if (crntAttr != Attribute::none) {
			*result = crntAttr;
			success = true;
		}
	}

	return success;
}

bool getElementAttr(tinyxml2::XMLElement* elem, const string& name, Element* result)
{
	return getEnumAttr(elem, name, elementNameMap, result);
}

bool getStringAttr(tinyxml2::XMLElement* elem, const string& name, string* result)
{
	const char* attr = elem->Attribute(name.c_str());

	if (attr)
		*result = string(attr);

	return attr;
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
			log1("Unable to parse XML attribute %s", name);
		}
	}
	return false;
}

bool getDamageInfo(tinyxml2::XMLElement* elem, DamageInfo* result)
{
	getNumericAttr(elem, "damage", &result->mag);
	getNumericAttr(elem, "knockback", &result->knockback);
	getElementAttr(elem, "element", &result->element);

	return result->mag > 0.0f;
}

void logXmlError(tinyxml2::XMLDocument* doc, string filename)
{
    tinyxml2::XMLError  error = doc->ErrorID();
//    int errorNum = static_cast<int>(error);
    string str1;
    string str2;
    string errorName;
    
    if(error < xmlErrors.size()){
        errorName = xmlErrors.at(error);
    }
    
    if(doc->GetErrorStr1()){
        str1 = doc->GetErrorStr1();
    }

    if(doc->GetErrorStr2()){
        str2 = doc->GetErrorStr2();
    }
    
    if(error){
        log5(
            "XML file %s error %d (%s): %s, %s\n",
            filename,
            error,
            errorName,
            str1,
            str2
        );
    }
}

bool autoName(tinyxml2::XMLElement* elem, string& field)
{
	bool result = field == "auto";
	if (result) {
		field = elem->Name();
	}
	return result;
}

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<area_properties>* result)
{
	area_properties props;

	props.sceneName = elem->Name();
	getStringAttr(elem, "next", &props.next);
	if (props.next == "none") props.next.clear();

	getColorAttr(elem, "ambient_light", &props.ambientLight);

	tinyxml2::XMLElement* rooms = elem->FirstChildElement();

	if (rooms)
	{
		//load multi-map

		SpaceVect roomSize;
		getVector(rooms, "size", &roomSize);

		string prefix;
		getStringAttr(rooms, "prefix", &prefix);

		for (
			tinyxml2::XMLElement* r = rooms->FirstChildElement();
			r != nullptr;
			r = r->NextSiblingElement()
		) {
			string roomMapName = prefix + string(r->Name());
			IntVec2 offset;
			SpaceVect temp;

			if (getVector(r, "offset", &temp)) {
				props.maps.push_back(make_pair(roomMapName, IntVec2(temp.x, temp.y)));
			}
			else if (getVector(r, "tile_offset", &temp)) {
				props.maps.push_back(make_pair(roomMapName, IntVec2(temp.x * roomSize.x, temp.y * roomSize.y)));

				if (roomSize.isZero()) {
					log2("areas.xml: %s:%s: tile_offset when no room size!", props.sceneName, roomMapName);
				}
			}
		}
	}
	else
	{
		//load single room
		string _map;
		getStringAttr(elem, "map", &_map);
		autoName(elem, _map);

		props.maps.push_back(make_pair(_map, IntVec2(0,0)));
	}

    *result = make_shared<area_properties>( props );
    return true;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<firepattern_properties>* _output)
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
		log1("fire pattern %s does not have a type.", elem->Name());
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

bool parseObject(tinyxml2::XMLElement* elem, IntVec2* result)
{
	string val;
	if (getStringAttr(elem, "size", &val)) {
		*result = toIntVector(val);
		return false;
	}
	return false;
}

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<sprite_properties>* result)
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
		log1("Sprite %s not found!", filename);
		return false;
	}

	parseObject(elem, &size);
	getColorAttr(elem, "color", &_color);
	getNumericAttr(elem, "dpi", &dpi);
	getNumericAttr(elem, "duration", &duration);
	getNumericAttr(elem, "ref-size", &referenceSize);

	*result = make_shared<sprite_properties>(sprite_properties{
		filename,
		size,
		dpi,
		referenceSize,
		duration,
		toColor3B(_color)
	});
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
        else if (type == "cone")
        {
            auto _result = make_shared<ConeLightArea>();
            
            getColorAttr(elem, "color", &_result->color);
            getNumericAttr(elem, "radius", &_result->radius);
            getNumericAttr(elem, "angularWidth", &_result->angleWidth);
            
            _result->angleWidth = toRads(_result->angleWidth);
            
            *result = _result;
            return true;
        }
	}

	return false;
}

}
