//
//  xml.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef xml_hpp
#define xml_hpp

namespace app {

	extern unordered_map<string, shared_ptr<area_properties>> areas;
	extern unordered_map<string, MagicEffectDescriptor*> effects;
	extern unordered_map<string, local_shared_ptr<firepattern_properties>> firePatterns;
	extern unordered_map<string, shared_ptr<LightArea>> lights;
	extern unordered_map<string, local_shared_ptr<object_properties>> objects;
	extern unordered_map<string, SpellDesc*> spells;
	extern unordered_map<string, shared_ptr<sprite_properties>> sprites;

	void loadAreas();
	void loadBombs();
	void loadBullets();
	void loadEffectAreas();
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

	shared_ptr<area_properties> getArea(const string& name);
	local_shared_ptr<bomb_properties> getBomb(const string& name);
	local_shared_ptr<bullet_properties> getBullet(const string& name);
	const MagicEffectDescriptor* getEffect(const string& name);
	local_shared_ptr<firepattern_properties> getFirePattern(const string& name);
	local_shared_ptr<item_properties> getItem(const string& name);
	shared_ptr<LightArea> getLight(const string& name);
	local_shared_ptr<agent_properties> getPlayer(const string& name);
	const SpellDesc* getSpell(const string& name);
	shared_ptr<sprite_properties> getSprite(const string& name);

	//A new bullet_properties will be stores at the given name. It will be copied from the
	//base if provided.
	local_shared_ptr<bullet_properties> addBullet(const string& name, const string& base);

}

#endif 
