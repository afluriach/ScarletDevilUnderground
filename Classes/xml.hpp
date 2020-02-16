//
//  xml.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef xml_hpp
#define xml_hpp

class agent_properties;
struct area_properties;
struct bomb_properties;
struct bullet_properties;
struct collectible_properties;
class enemy_properties;
class environment_object_properties;
struct firepattern_properties;
class item_properties;
class LightArea;
class MagicEffectDescriptor;
class npc_properties;
class object_properties;
class SpellDesc;

namespace app {
	typedef function< bool(tinyxml2::XMLElement*, local_shared_ptr<MagicEffectDescriptor>*)> effect_parser;
	typedef function< bool(tinyxml2::XMLElement*, local_shared_ptr<SpellDesc>*)> spell_parser;

	extern const unordered_map<string, spell_parser> spellParsers;
	extern const unordered_map<string, effect_parser> effectParsers;

	extern unordered_map<string, area_properties> areas;
	extern unordered_map<string, AttributeMap> attributes;
	extern unordered_map<string, local_shared_ptr<bomb_properties>> bombs;
	extern unordered_map<string, local_shared_ptr<bullet_properties>> bullets;
	extern unordered_map<string, collectible_properties> collectibles;
	extern unordered_map<string, local_shared_ptr<MagicEffectDescriptor>> effects;
	extern unordered_map<string, local_shared_ptr<enemy_properties>> enemies;
	extern unordered_map<string, local_shared_ptr<environment_object_properties>> environmentObjects;
	extern unordered_map<string, local_shared_ptr<firepattern_properties>> firePatterns;
	extern unordered_map<string, local_shared_ptr<floorsegment_properties>> floors;
	extern unordered_map<string, local_shared_ptr<item_properties>> items;
	extern unordered_map<string, boost::shared_ptr<LightArea>> lights;
	extern unordered_map<string, local_shared_ptr<npc_properties>> npc;
	extern unordered_map<string, local_shared_ptr<agent_properties>> players;
	extern unordered_map <string, local_shared_ptr<SpellDesc>> spells;
	extern unordered_map<string, sprite_properties> sprites;

	void loadAreas();
	void loadAttributes();
	void loadBombs();
	void loadBullets();
	void loadCollectibles();
	void loadEffects();
	void loadEnemies();
	void loadEnvironmentObjects();
	void loadFirePatterns();
	void loadFloors();
	void loadItems();
	void loadLights();
	void loadNPCs();
	void loadPlayers();
	void loadSpells();
	void loadSprites();

	area_properties getArea(const string& name);
	local_shared_ptr<bomb_properties> getBomb(const string& name);
	local_shared_ptr<bullet_properties> getBullet(const string& name);
	collectible_properties getCollectible(const string& name);
	local_shared_ptr<MagicEffectDescriptor> getEffect(const string& name);
	local_shared_ptr<enemy_properties> getEnemy(const string& name);
	local_shared_ptr<environment_object_properties> getEnvironemntObject(const string& name);
	local_shared_ptr<firepattern_properties> getFirePattern(const string& name);
	local_shared_ptr<floorsegment_properties> getFloor(const string& name);
	local_shared_ptr<item_properties> getItem(const string& name);
	boost::shared_ptr<LightArea> getLight(const string& name);
	local_shared_ptr<agent_properties> getNPC(const string& name);
	local_shared_ptr<agent_properties> getPlayer(const string& name);
	local_shared_ptr<SpellDesc> getSpell(const string& name);
	sprite_properties getSprite(const string& name);
	AttributeMap getAttributes(const string& name);

	template<typename T>
	inline void loadObjects(string filename, unordered_map<string,T>& _map)
	{
		tinyxml2::XMLDocument objects;
		auto error = objects.Parse(io::loadTextFile(filename).c_str());

		if (error != tinyxml2::XML_SUCCESS) {
			log("XML error: %d", error);
			return;
		}

		tinyxml2::XMLNode* root = objects.FirstChild();

		for (
			tinyxml2::XMLElement* crnt = root->FirstChildElement();
			crnt != nullptr;
			crnt = crnt->NextSiblingElement() )
		{
			T object;
			if (parseObject(crnt, &object)) {
				_map.insert_or_assign(crnt->Name(), object);
			}
			else {
				log("%s : %s failed to load!", filename, crnt->Name());
			}
		}
	}

	template<typename T>
	inline void loadObjectsShared(string filename, unordered_map<string, local_shared_ptr<T>>& _map)
	{
		tinyxml2::XMLDocument objects;
		auto error = objects.Parse(io::loadTextFile(filename).c_str());

		if (error != tinyxml2::XML_SUCCESS) {
			log("XML error: %d", error);
			return;
		}

		tinyxml2::XMLNode* root = objects.FirstChild();

		for (
			tinyxml2::XMLElement* crnt = root->FirstChildElement();
			crnt != nullptr;
			crnt = crnt->NextSiblingElement())
		{
			local_shared_ptr<T> object = make_local_shared<T>();
			if (parseObject(crnt, object)) {
				_map.insert_or_assign(crnt->Name(), object);
			}
			else {
				log("%s : %s failed to load!", filename, crnt->Name());
			}
		}
	}

	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<object_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<environment_object_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<agent_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, area_properties* result);
	bool parseObject(tinyxml2::XMLElement* elem, AttributeMap* result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<enemy_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<npc_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<firepattern_properties>* result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<floorsegment_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, sprite_properties* result);
	bool parseObject(tinyxml2::XMLElement* elem, boost::shared_ptr<LightArea>* result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<bullet_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<bomb_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<MagicEffectDescriptor>* result);
	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<item_properties> result);
	bool parseObject(tinyxml2::XMLElement* elem, collectible_properties* result);
	bool parseObject(tinyxml2::XMLElement * elem, local_shared_ptr<SpellDesc>* result);

	bool getAttributeAttr(tinyxml2::XMLElement* elem, const string& name, Attribute* result);
	bool getStringAttr(tinyxml2::XMLElement* elem, const string& name, string* result);
	bool getColorAttr(tinyxml2::XMLElement* elem, const string& name, Color4F* result);
	bool getVector(tinyxml2::XMLElement* elem, const string& name, SpaceVect* result);
	bool getDamageInfo(tinyxml2::XMLElement* elem, DamageInfo* result);

	template<typename T>
	bool copyBaseObject(
		tinyxml2::XMLElement* elem,
		unordered_map<string, local_shared_ptr<T>>& _map,
		T* output
	) {
		string base;
		if (getStringAttr(elem, "base", &base)) {
			auto _base = getOrDefault(_map, base);
			if (_base) {
				*output = *_base;
				return true;
			}
			else {
				log("%s: unknown base %s type: %s", elem->Name(), typeid(T).name(), base);
			}
		}
		return false;
	}

	template<typename T>
	bool copyBaseObjectShared(
		tinyxml2::XMLElement * elem,
		unordered_map<string, local_shared_ptr<T>> & _map,
		local_shared_ptr<T> output
	) {
		string base;
		if (getStringAttr(elem, "base", &base)) {
			local_shared_ptr<T> _base = getOrDefault(_map, base);
			if (_base) {
				*output = *_base;
				return true;
				
			}
			else {
				log("%s: unknown base %s type: %s", elem->Name(), typeid(T).name(), base);
				
			}
			
		}
		return false;
		
	}

	template<typename T>
	bool getSubObject(
		tinyxml2::XMLElement* elem,
		string fieldName,
		local_shared_ptr<T>* result,
		const unordered_map<string, local_shared_ptr<T>>& _map,
		bool autoName
	) {
		string field;
		getStringAttr(elem, fieldName, &field);

		if (autoName && field == "auto") {
			field = elem->Name();
		}

		auto it = _map.find(field);

		if (it != _map.end()) {
			*result = it->second;
		}

		return it != _map.end();
	}

	template<typename T>
	bool getSubObject(
		tinyxml2::XMLElement* elem,
		string fieldName,
		boost::shared_ptr<T>* result,
		const unordered_map<string, boost::shared_ptr<T>>& _map,
		bool autoName
	) {
		string field;
		getStringAttr(elem, fieldName, &field);

		if (autoName && field == "auto") {
			field = elem->Name();
		}

		auto it = _map.find(field);

		if (it != _map.end()) {
			*result = it->second;
		}

		return it != _map.end();
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
}

#endif 
