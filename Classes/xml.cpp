//
//  xml.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "Bomb.hpp"
#include "Collectibles.hpp"
#include "Enemy.hpp"
#include "FileIO.hpp"
#include "FirePatternImpl.hpp"
#include "Graphics.h"
#include "Item.hpp"
#include "NPC.hpp"
#include "PlayScene.hpp"

namespace app {

unordered_map<string, area_properties> areas;
unordered_map<string, AttributeMap> attributes;
unordered_map<string, shared_ptr<bomb_properties>> bombs;
unordered_map<string, shared_ptr<bullet_properties>> bullets;
unordered_map<string, collectible_properties> collectibles;
unordered_map<string, shared_ptr<MagicEffectDescriptor>> effects;
unordered_map<string, shared_ptr<enemy_properties>> enemies;
unordered_map<string, shared_ptr<firepattern_properties>> firePatterns;
unordered_map<string, floorsegment_properties> floors;
unordered_map<string, shared_ptr<item_properties>> items;
unordered_map<string, shared_ptr<LightArea>> lights;
unordered_map<string, shared_ptr<npc_properties>> npc;
unordered_map<string, shared_ptr<agent_properties>> players;
unordered_map<string, sprite_properties> sprites;

GObject::AdapterType enemyAdapter(shared_ptr<enemy_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		agent_attributes attr = Agent::parseAttributes(args);
	
		if (Agent::conditionalLoad(space, attr, props)) {
			return new Enemy(space, id, attr, props);
		}
		else {
			return nullptr;
		}
	};
}

GObject::AdapterType npcAdapter(shared_ptr<npc_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		agent_attributes attr = Agent::parseAttributes(args);

		if (Agent::conditionalLoad(space, attr, props)) {
			return new NPC(space, id, attr, props);
		}
		else {
			return nullptr;
		}
	};
}

GObject::AdapterType itemAdapter(shared_ptr<item_properties> props)
{
	return [props](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		item_attributes attr = Item::parseAttributes(args);

		if (Item::conditionalLoad(space, attr, props)) {
			return new Item(space, id, attr, props);
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

void loadAttributes()
{
	loadObjects<AttributeMap>("objects/attribute_sets.xml", app::attributes);
}

void loadBombs()
{
	loadObjectsShared<bomb_properties>("objects/bombs.xml", app::bombs);
}

void loadBullets()
{
	loadObjectsShared<bullet_properties>("objects/bullets.xml", app::bullets);
}

void loadCollectibles()
{
	loadObjects<collectible_properties>("objects/collectibles.xml", app::collectibles);
}

void loadEffects()
{
	loadObjects<shared_ptr<MagicEffectDescriptor>>("objects/magic-effects.xml", app::effects);
}

void loadEnemies()
{
	loadObjectsShared<enemy_properties>("objects/enemies.xml", app::enemies);

	for (auto entry : enemies){
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

void loadItems()
{
	loadObjectsShared<item_properties>("objects/items.xml", app::items);

	for (auto entry : items) {
		GObject::namedObjectTypes.insert_or_assign(entry.first, itemAdapter(entry.second));
	}
}

void loadLights()
{
	loadObjects<shared_ptr<LightArea>>("objects/lights.xml", app::lights);
}

void loadNPCs()
{
	loadObjectsShared<npc_properties>("objects/npc.xml", app::npc);

	for (auto entry : npc){
		GObject::namedObjectTypes.insert_or_assign(entry.first, npcAdapter(entry.second));
	}
}

void loadPlayers()
{
	loadObjectsShared<agent_properties>("objects/players.xml", app::players);
}

void loadSprites()
{
	loadObjects<sprite_properties>("objects/sprites.xml", app::sprites);
}

area_properties getArea(const string& name)
{
	return getOrDefault(areas, name);
}

shared_ptr<bomb_properties> getBomb(const string& name)
{
	return getOrDefault(bombs, name);
}

shared_ptr<bullet_properties> getBullet(const string& name)
{
	return getOrDefault(bullets, name);
}

collectible_properties getCollectible(const string& name)
{
	return getOrDefault(collectibles, name);
}

shared_ptr<MagicEffectDescriptor> getEffect(const string& name)
{
	return getOrDefault(effects, name);
}

shared_ptr<enemy_properties> getEnemy(const string& name)
{
	return getOrDefault(enemies, name);
}

shared_ptr<firepattern_properties> getFirePattern(const string& name)
{
	return getOrDefault(firePatterns, name);
}

shared_ptr<item_properties> getItem(const string& name)
{
	return getOrDefault(items, name);
}

shared_ptr<LightArea> getLight(const string& name)
{
	return getOrDefault(lights, name);
}

shared_ptr<agent_properties> getNPC(const string& name)
{
	return getOrDefault(npc, name);
}

shared_ptr<agent_properties> getPlayer(const string& name)
{
	return getOrDefault(players, name);
}

sprite_properties getSprite(const string& name)
{
	return getOrDefault(sprites, name);
}

AttributeMap getAttributes(const string& name)
{
	return getOrDefault(attributes, name);
}

//get Attribute attribute [sic]
bool getAttributeAttr(tinyxml2::XMLElement* elem, const string& name, Attribute* result)
{
	const char* attr = elem->Attribute(name.c_str());
	bool success = false;

	if (attr) {
		Attribute crntAttr = AttributeSystem::getAttribute(attr);
		if (crntAttr != Attribute::end) {
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

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<agent_properties> result)
{
	result->typeName = elem->Name();
	result->radius = Agent::defaultSize;

	getStringAttr(elem, "name", &result->name);
	getStringAttr(elem, "sprite", &result->sprite);
	getStringAttr(elem, "attributes", &result->attributes);
	getStringAttr(elem, "ai_package", &result->ai_package);
	getStringAttr(elem, "effects", &result->effects);

	getSubObject(elem, "light", &result->lightSource, lights, true);

	autoName(elem, result->sprite);
	autoName(elem, result->attributes);
	autoName(elem, result->ai_package);

	getNumericAttr(elem, "radius", &result->radius);
	getNumericAttr(elem, "mass", &result->mass);
	getNumericAttr(elem, "viewAngle", &result->viewAngle);
	getNumericAttr(elem, "viewRange", &result->viewRange);

	getNumericAttr(elem, "detectEssence", &result->detectEssence);
	getNumericAttr(elem, "isFlying", &result->isFlying);

	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<enemy_properties> result)
{
	parseObject(elem, static_cast<shared_ptr<agent_properties>>(result));

	getStringAttr(elem, "firepattern", &result->firepattern);
	getStringAttr(elem, "collectible", &result->collectible);

	getDamageInfo(elem, &result->touchEffect);
	result->touchEffect.type = DamageType::touch;

	return true;
}

bool parseDialogs(tinyxml2::XMLElement* elem, list<shared_ptr<dialog_entry>>& result)
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

		auto entry = make_shared<dialog_entry>();
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

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<npc_properties> result)
{
	parseObject(elem, static_cast<shared_ptr<agent_properties>>(result));

	tinyxml2::XMLElement* dialogs = elem->FirstChildElement("dialogs");
	if (dialogs) {
		parseDialogs(dialogs, result->dialogs);
	}

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

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<bullet_properties> result)
{
	bullet_properties props{
		0.0,
		SpaceVect::zero,
		DamageInfo(0.0f, DamageType::bullet),
		string(),
		string(),
		1,
		0,
		true,
		false,
		false
	};
	
	copyBaseObject(elem, bullets, &props);

	getNumericAttr(elem, "speed", &props.speed);
	if (!getVector(elem, "dimensions", &props.dimensions)) {
		getNumericAttr(elem, "radius", &props.dimensions.x);
	}

	getDamageInfo(elem, &props.damage);

	getStringAttr(elem, "sprite", &props.sprite);
	getStringAttr(elem, "lightSource", &props.lightSource);
	getNumericAttr(elem, "hitCount", &props.hitCount);
	getNumericAttr(elem, "ricochet", &props.ricochetCount);
	getNumericAttr(elem, "directionalLaunch", &props.directionalLaunch);
	getNumericAttr(elem, "ignoreObstacles", &props.ignoreObstacles);
	getNumericAttr(elem, "deflectBullets", &props.deflectBullets);

	*result = props;
	return true;
}

bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<bomb_properties> result)
{
	bomb_properties props = {
		"",
		"",
		1.0f / 16.0f,
		0.0,
		0.0,
		0.0f,
		DamageInfo(0.0f, DamageType::bomb)
	};

	getStringAttr(elem, "sprite", &props.sprite);
	getStringAttr(elem, "exposionSound", &props.explosionSound);

	getNumericAttr(elem, "friction", &props.friction);
	getNumericAttr(elem, "blastRadius", &props.blastRadius);
	getNumericAttr(elem, "fuseTime", &props.fuseTime);
	getNumericAttr(elem, "cost", &props.cost);

	getDamageInfo(elem, &props.damage);

	if (props.blastRadius <= 0.0f || props.fuseTime <= 0.0f) {
		log("bomb properties missing");
		return false;
	}
	else {
		*result = props;
		return true;
	}
}

bool parseObject(tinyxml2::XMLElement * elem, shared_ptr<item_properties> result)
 {
	copyBaseObjectShared(elem, items, result);	
	
	result->name = elem->Name();

	getStringAttr(elem, "cls", &result->scriptName);
	autoName(elem, result->scriptName);

	getStringAttr(elem, "sprite", &result->sprite);
	autoName(elem, result->sprite);

	getSubObject(elem, "light", &result->light, app::lights, true);

	getStringAttr(elem, "dialog", &result->onAcquireDialog);

	return true;
 }

bool parseObject(tinyxml2::XMLElement* elem, collectible_properties* result)
{
	collectible_properties coll;
	string effect;

	coll.magnitude = 0.0f;
	coll.length = 0.0f;

	getStringAttr(elem, "sprite", &coll.sprite);
	getStringAttr(elem, "effect", &effect);
	getNumericAttr(elem, "magnitude", &coll.magnitude);
	getNumericAttr(elem, "length", &coll.length);

	autoName(elem, coll.sprite);

	coll.effect = getEffect(effect);

	if (coll.effect) {
		*result = coll;
	}

	return static_cast<bool>(coll.effect);
}

}
