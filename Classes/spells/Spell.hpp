//
//  Spell.hpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

#include "spell_types.hpp"

#define STANDARD_CONS(name) inline name(GObject* caster) : Spell(caster) {}
#define GET_DESC(name) virtual inline const SpellDesc* getDescriptor() { return Spell::getDescriptorByName(#name); }

class Bullet;
class GObject;
class MagicEffectDescriptor;
class SpellDesc;
class Torch;

class Spell
{
public:
	friend class GObject;
	friend class SpellSystem;

    static unordered_map<string, const SpellDesc*> spellDescriptors;
	static const vector<string> playerSpells;
	static const vector<string> playerPowerAttacks;

	static const SpellDesc* getDescriptorByName(const string& name);

	static void initDescriptors();

	//length: -1 means indefinite, 0 means immediate
	//updateInterval: -1 means no update, 0 means every frame, units in seconds.
	Spell(GObject* caster, const SpellDesc* desc, unsigned int id, spell_params params);
	virtual ~Spell();
    
	template<class T>
	inline T* getCasterAs() const{
		return dynamic_cast<T*>(caster);
	}

	GSpace* getSpace() const;

	bullet_attributes getBulletAttributes(local_shared_ptr<bullet_properties> props) const;

	gobject_ref spawnBullet(
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceVect velocity,
		SpaceFloat angle,
		SpaceFloat angularVelocity
	);
	gobject_ref launchBullet(
		local_shared_ptr<bullet_properties> props,
		SpaceVect displacement,
		SpaceFloat angle,
		SpaceFloat angularVelocity = 0.0,
		bool obstacleCheck = true
	);

	inline const SpellDesc* getDescriptor() const { return descriptor; }
	unsigned int getID() const;
	string getName() const;
	spell_cost getCost() const;

	inline virtual void init() {}
	inline virtual void update() {}
	inline virtual void end() {}
	inline virtual void onBulletRemove(Bullet* b) {}

	void stop();
protected:
	void runUpdate();

	//spell_params
	spell_cost _cost;
	SpaceFloat length;
	SpaceFloat updateInterval;

	SpaceFloat t = 0.0;
	SpaceFloat lastUpdate = 0.0;

	const SpellDesc* descriptor;
    GObject* caster;
	unsigned int id;
};

#endif /* Spell_hpp */
