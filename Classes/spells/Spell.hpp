//
//  Spell.hpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#ifndef Spell_hpp
#define Spell_hpp

#define STANDARD_CONS(name) inline name(GObject* caster) : Spell(caster) {}
#define GET_DESC(name) virtual inline const SpellDesc* getDescriptor() { return app::getSpell(#name); }

class Spell
{
public:
	friend class GObject;
	friend class SpellSystem;

	//length: -1 means indefinite, 0 means immediate
	//updateInterval: -1 means no update, 0 means every frame, units in seconds.
	Spell(GObject* caster, const SpellDesc* desc, unsigned int id);
	virtual ~Spell();
    
	template<class T>
	inline T* getCasterAs() const{
		return dynamic_cast<T*>(caster);
	}

	GSpace* getSpace() const;

	bullet_attributes getBulletAttributes(local_shared_ptr<bullet_properties> props);

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

	vector<gobject_ref> spawnBulletRadius(
		local_shared_ptr<bullet_properties> props,
		SpaceFloat displacement,
		int count
	);

	void bulletCircle(
		const vector<gobject_ref>& bullets,
		SpaceFloat distance,
		SpaceFloat angularPos
	);

	inline const SpellDesc* getDescriptor() const { return descriptor; }
	unsigned int getID() const;
	string getName() const;
	spell_cost getCost() const;
	SpaceFloat getTime() const;

	inline bool isSpellActive() const { return active; }

	inline virtual void init() {}
	inline virtual void update() {}
	inline virtual void end() {}
	inline virtual void onBulletRemove(Bullet* b) {}

	void stop();

	int _refcount = 0;
protected:
	void runUpdate();

	SpaceFloat t = 0.0;
	SpaceFloat lastUpdate = 0.0;

	const SpellDesc* descriptor;
    GObject* caster;
	unsigned int id;
	//set to false when spell is stopped and removed from SpellSystem
	bool active = true;
};

#endif /* Spell_hpp */
