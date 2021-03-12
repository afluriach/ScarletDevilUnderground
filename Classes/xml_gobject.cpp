//
//  xml_gobject.cpp
//  Koumachika
//
//  Created by Toni on 3/4/21.
//
//

#include "Prefix.h"

#include "Bomb.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnvironmentObject.hpp"
#include "FloorSegment.hpp"
#include "Item.hpp"
#include "NPC.hpp"

namespace app {

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
			*f = [name](NPC* n) -> void {
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


	bool parseObject(tinyxml2::XMLElement* elem, AttributeMap* result)
	{
		AttributeMap _result;

		for (
			tinyxml2::XMLElement* crnt = elem->FirstChildElement();
			crnt != nullptr;
			crnt = crnt->NextSiblingElement()
			) {
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

	void parseAgentEffectsList(tinyxml2::XMLElement* elem, list<effect_entry>& effects)
	{
		for (
			tinyxml2::XMLElement* e = elem->FirstChildElement();
			e != nullptr;
			e = e->NextSiblingElement()
		) {
			string effectName = e->Name();
			const MagicEffectDescriptor* desc = app::getEffect(effectName);
			effect_attributes attr;
			attr.length = -1.0f;

			if (!desc) {
				log("parseAgentEffectsList: unknown effect " + effectName);
				continue;
			}

			getNumericAttr(e, "magnitude", &attr.magnitude);
			getNumericAttr(e, "radius", &attr.radius);

			effects.push_back(make_pair(desc, attr));
		}
	}

	void parseSpellInventory(tinyxml2::XMLElement* elem, list<const SpellDesc*>& spells)
	{
		for (
			tinyxml2::XMLElement* e = elem->FirstChildElement();
			e != nullptr;
			e = e->NextSiblingElement()
		) {
			string name = e->Name();
			const SpellDesc* desc = app::getSpell(name);

			if (!desc) {
				log("parseSpellInventory: unknown spell " + name);
				continue;
			}

			spells.push_back(desc);
		}
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

		tinyxml2::XMLElement* effects = elem->FirstChildElement("effects");
		if (effects){
			parseAgentEffectsList(effects, result->effects);
		}

		tinyxml2::XMLElement* spells = elem->FirstChildElement("spells");
		if (spells) {
			parseSpellInventory(spells, result->spellInventory);
		}

		getSubObjectPtr<SpellDesc>(elem, "attack", &result->attack, app::spells, true);

		getStringAttr(elem, "ai_package", &result->ai_package);

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

	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<floorsegment_properties> result)
	{
		parseObject(elem, static_cast<local_shared_ptr<object_properties>>(result));

		getStringAttr(elem, "sfx", &result->sfxRes);
		getStringAttr(elem, "sprite", &result->sprite);

		getNumericAttr(elem, "traction", &result->traction);

		getNumericAttr(elem, "pressurePlate", &result->pressurePlate);

		return true;
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

	bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<item_properties> result)
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
