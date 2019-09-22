//
//  xml.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef xml_hpp
#define xml_hpp

struct bomb_properties;
struct bullet_properties;
struct enemy_properties;
struct firepattern_properties;
class LightArea;

namespace app {
	extern unordered_map<string, AttributeMap> attributes;
	extern unordered_map<string, shared_ptr<bomb_properties>> bombs;
	extern unordered_map<string, shared_ptr<bullet_properties>> bullets;
	extern unordered_map<string, shared_ptr<enemy_properties>> enemies;
	extern unordered_map<string, shared_ptr<firepattern_properties>> firePatterns;
	extern unordered_map<string, floorsegment_properties> floors;
	extern unordered_map<string, shared_ptr<LightArea>> lights;
	extern unordered_map<string, sprite_properties> sprites;

	void loadAttributes();
	void loadBombs();
	void loadBullets();
	void loadEnemies();
	void loadFirePatterns();
	void loadFloors();
	void loadLights();
	void loadSprites();

	shared_ptr<bomb_properties> getBomb(const string& name);
	shared_ptr<bullet_properties> getBullet(const string& name);
	shared_ptr<enemy_properties> getEnemy(const string& name);
	shared_ptr<firepattern_properties> getFirePattern(const string& name);
	shared_ptr<LightArea> getLight(const string& name);
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

	DamageInfo getDamageInfo(tinyxml2::XMLElement* elem, DamageType type);

	bool parseObject(tinyxml2::XMLElement* elem, AttributeMap* result);
	bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<enemy_properties>* result);
	bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<firepattern_properties>* result);
	bool parseObject(tinyxml2::XMLElement* elem, floorsegment_properties* result);
	bool parseObject(tinyxml2::XMLElement* elem, sprite_properties* result);
	bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<LightArea>* result);
	bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<bullet_properties>* result);
	bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<bomb_properties>* result);
}

#endif 
