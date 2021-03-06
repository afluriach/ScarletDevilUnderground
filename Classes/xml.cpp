//
//  xml.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "Bomb.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnvironmentObject.hpp"
#include "FileIO.hpp"
#include "FirePatternImpl.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "Item.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "NPC.hpp"
#include "PlayScene.hpp"
#include "SpellDescriptor.hpp"

namespace app {

unordered_map<string, area_properties> areas;
unordered_map<string, MagicEffectDescriptor*> effects;
unordered_map<string, local_shared_ptr<firepattern_properties>> firePatterns;
unordered_map<string, local_shared_ptr<floorsegment_properties>> floors;
unordered_map<string, shared_ptr<LightArea>> lights;
unordered_map<string, local_shared_ptr<object_properties>> objects;
unordered_map<string, SpellDesc*> spells;
unordered_map<string, shared_ptr<sprite_properties>> sprites;

GObject::AdapterType objectAdapter(local_shared_ptr<agent_properties> props)
{
	return nullptr;
}

GObject::AdapterType objectAdapter(local_shared_ptr<bomb_properties> props)
{
	return nullptr;
}

GObject::AdapterType objectAdapter(local_shared_ptr<bullet_properties> props)
{
	return nullptr;
}

GObject::AdapterType objectAdapter(local_shared_ptr<effectarea_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		object_params params(args);

		return allocator_new<EffectArea>(space, id, params, props);
	};
}

GObject::AdapterType objectAdapter(local_shared_ptr<enemy_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		object_params params(args);
	
		if (Agent::conditionalLoad(space, params, props)) {
			return allocator_new<Enemy>(space, id, params, props);
		}
		else {
			return nullptr;
		}
	};
}

GObject::AdapterType objectAdapter(local_shared_ptr<npc_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		object_params params(args);

		if (Agent::conditionalLoad(space, params, props)) {
			return allocator_new<NPC>(space, id, params, props);
		}
		else {
			return nullptr;
		}
	};
}

GObject::AdapterType objectAdapter(local_shared_ptr<item_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		object_params params(args);

		if (Item::conditionalLoad(space, params, props)) {
			return allocator_new<Item>(space, id, params, props);
		}
		else {
			return nullptr;
		}
	};
}

GObject::AdapterType objectAdapter(local_shared_ptr<floorsegment_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		return allocator_new<FloorSegment>(space, id, args, props);
	};
}

GObject::AdapterType objectAdapter(local_shared_ptr<environment_object_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		if (EnvironmentObject::conditionalLoad(space, id, args, props)) {
			return allocator_new<EnvironmentObject>(space, id, args, props);
		}
		else {
			return nullptr;
		}
	};
}

void loadAreas()
{
	loadObjects<area_properties>("objects/areas.xml", app::areas);
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

void loadSprites()
{
	loadObjects<shared_ptr<sprite_properties>>("objects/sprites.xml", app::sprites);
}

area_properties getArea(const string& name)
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

local_shared_ptr<item_properties> getItem(const string& name)
{
	return getObjectProperties<item_properties>(name);
}

shared_ptr<LightArea> getLight(const string& name)
{
	return getOrDefault(lights, name);
}

local_shared_ptr<agent_properties> getPlayer(const string& name)
{
	return getObjectProperties<agent_properties>(name);
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
			log("addBullet(%s,%s): base not found", name, base);
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
			log("Unable to parse XML attribute %s", name);
		}
	}
	return false;
}

bool getDamageInfo(tinyxml2::XMLElement* elem, DamageInfo* result)
{
	getNumericAttr(elem, "damage", &result->mag);
	getNumericAttr(elem, "knockback", &result->knockback);
	getAttributeAttr(elem, "element", &result->element);

	return result->mag > 0.0f;
}

bool autoName(tinyxml2::XMLElement* elem, string& field)
{
	bool result = field == "auto";
	if (result) {
		field = elem->Name();
	}
	return result;
}

bool parseCondition(string s, function<bool(NPC*)>* f)
{
	vector<string> tokens = splitString(s, ":");

	if (tokens.size() != 2) {
		log("Invalid condition string: %s", s);
		return false;
	}

	if (tokens[0] == "hasItem") {
		string item = tokens[1];
		*f = [item](NPC* n) -> bool {
			return App::crntState->hasItem(item);
		};
		return true;
	}
	else if (tokens[0] == "scriptMethod") {
		string name = tokens[1];
		*f = [name](NPC* n) -> bool {
			if (!n->hasMethod(name)) {
				log("condition scriptMethod %s does not exist!", name);
				return false;
			}
			else {
				return n->runScriptMethod<bool>(name);
			}
		};
		return true;
	}
	else if (tokens[0] == "attributeEQ") {
		vector<string> t = splitString(tokens[1], ",");
		string attribute = t[0];
		int val = boost::lexical_cast<int>(t[1]);
		*f = [attribute, val](NPC* n) -> bool {
			return App::crntState->getAttribute(attribute) == val;
		};
		return true;
	}
	else {
		log("Unknown condition function %s!", tokens[0]);
	}

	return false;
}

bool parseEffect(string s, function<void(NPC*)>* f)
{
	vector<string> tokens = splitString(s, ":");

	if (tokens.size() != 2) {
		log("Invalid condition string: %s", s);
		return false;
	}

	if (tokens[0] == "registerChamberAvailable") {
		string chamber = tokens[1];
		*f = [chamber](NPC* n) -> void {
			App::crntState->registerChamberAvailable(chamber);
		};
		return true;
	}
	else if (tokens[0] == "scriptMethod") {
		string name = tokens[1];
		*f = [name](NPC* n) -> void  {
			if (!n->hasMethod(name)) {
				log("effect scriptMethod %s does not exist!", name);
			}
			else {
				n->runVoidScriptMethod(name);
			}
		};
		return true;
	}
	else if (tokens[0] == "addItem") {
		string item = tokens[1];
		*f = [item](NPC* n) -> void {
			App::crntState->addItem(item);
		};
		return true;
	}
	else {
		log("Unknown effect function %s!", tokens[0]);
	}

	return false;
}


bool parseObject(tinyxml2::XMLElement* elem, area_properties* result)
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
					log("areas.xml: %s:%s: tile_offset when no room size!", props.sceneName, roomMapName);
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

	*result = props;
	return true;
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

		if (crntAttr != Attribute::none && getNumericAttr(crnt, "val", &val)) {
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

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<object_properties> result)
{
	result->clsName = elem->Name();
	getStringAttr(elem, "name", &result->properName);

	if (!getVector(elem, "dimensions", &result->dimensions)) {
		getNumericAttr(elem, "radius", &result->dimensions.x);
	}
	getNumericAttr(elem, "mass", &result->mass);
	getNumericAttr(elem, "friction", &result->friction);

	getSubObject(elem, "light", &result->light, lights, true);
	getSubObject(elem, "sprite", &result->sprite, sprites, true);

	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<environment_object_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	getStringAttr(elem, "cls", &result->scriptName);
	autoName(elem, result->scriptName);

	getStringAttr(elem, "interactionIcon", &result->interactionIcon);
	getNumericAttr(elem, "interactible", &result->interactible);

	string layers;
	getStringAttr(elem, "layers", &layers);
	result->layers = parseLayers(layers);
	if (result->layers == PhysicsLayers::none) {
		if (!layers.empty()) {
			log("invalid object layers: %s", layers);
		}
		result->layers = PhysicsLayers::onGround;
	}

	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<effectarea_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	string effectName;
	float magnitude = 0.0f;

	getNumericAttr(elem, "magnitude", &magnitude);
	getStringAttr(elem, "effect", &effectName);

	if (effectName.size() > 0) {
		auto e = app::getEffect(effectName);
		if (e) {
			result->effect = e;
			result->magnitude = magnitude;
			return true;
		}
	}

	return false;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<agent_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	if (result->dimensions.x == 0.0) {
		result->dimensions.x = Agent::defaultSize;
	}

	tinyxml2::XMLElement* attributes = elem->FirstChildElement("attributes");
	if (attributes) {
		parseObject(attributes, &result->attributes);
	}

	getStringAttr(elem, "ai_package", &result->ai_package);
	getStringAttr(elem, "effects", &result->effects);

	autoName(elem, result->ai_package);

	getNumericAttr(elem, "viewAngle", &result->viewAngle);
	getNumericAttr(elem, "viewRange", &result->viewRange);

	getNumericAttr(elem, "detectEssence", &result->detectEssence);
	getNumericAttr(elem, "isFlying", &result->isFlying);

	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<enemy_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<agent_properties>>(result));

	getStringAttr(elem, "firepattern", &result->firepattern);
	getStringAttr(elem, "collectible", &result->collectible);

	getDamageInfo(elem, &result->touchEffect);
	result->touchEffect.type = DamageType::touch;

	return true;
}

bool parseDialogs(tinyxml2::XMLElement* elem, list<local_shared_ptr<dialog_entry>>& result)
{
	if (!elem) return false;

	for (
		tinyxml2::XMLElement* d = elem->FirstChildElement();
		d != nullptr;
		d = d->NextSiblingElement()
	) {
		string condition;
		string effect;
		getStringAttr(d, "condition", &condition);
		getStringAttr(d, "effect", &effect);

		auto entry = make_local_shared<dialog_entry>();
		entry->dialog = string(d->Name());

		if (condition.size() > 0)
			parseCondition(condition, &entry->condition);
		if (effect.size() > 0)
			parseEffect(effect, &entry->effect);

		getNumericAttr(d, "once", &entry->once);

		result.push_back(entry);
	}

	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<npc_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<agent_properties>>(result));

	tinyxml2::XMLElement* dialogs = elem->FirstChildElement("dialogs");
	if (dialogs) {
		parseDialogs(dialogs, result->dialogs);
	}

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

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<floorsegment_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	getStringAttr(elem, "sfx", &result->sfxRes);
	getStringAttr(elem, "sprite", &result->sprite);

	getNumericAttr(elem, "traction", &result->traction);

	getNumericAttr(elem, "pressurePlate", &result->pressurePlate);

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
		log("Sprite %s not found!", filename);
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
	}

	return false;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<bullet_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	result->damage.type = DamageType::bullet;

	getNumericAttr(elem, "speed", &result->speed);
	getDamageInfo(elem, &result->damage);

	getNumericAttr(elem, "hitCount", &result->hitCount);
	getNumericAttr(elem, "ricochet", &result->ricochetCount);
	getNumericAttr(elem, "invisible", &result->invisible);
	getNumericAttr(elem, "ignoreObstacles", &result->ignoreObstacles);
	getNumericAttr(elem, "deflectBullets", &result->deflectBullets);

	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<bomb_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	getStringAttr(elem, "explosionSound", &result->explosionSound);

	getNumericAttr(elem, "blastRadius", &result->blastRadius);
	getNumericAttr(elem, "fuseTime", &result->fuseTime);
	getNumericAttr(elem, "cost", &result->cost);

	getDamageInfo(elem, &result->damage);

	if (result->blastRadius <= 0.0f || result->fuseTime <= 0.0f) {
		log("bomb properties missing");
		return false;
	}
	else {
		return true;
	}
}

bool parseObject(tinyxml2::XMLElement * elem, local_shared_ptr<item_properties> result)
{
	parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

	if (result->dimensions.x == 0.0) {
		result->dimensions.x = 0.5;
	}

	getStringAttr(elem, "cls", &result->scriptName);
	autoName(elem, result->scriptName);

	getStringAttr(elem, "dialog", &result->onAcquireDialog);

	getNumericAttr(elem, "addToInventory", &result->addToInventory);

	return true;
}

}
