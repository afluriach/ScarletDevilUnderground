//
//  object_properties.hpp
//  Koumachika
//
//  Created by Toni on 1/23/24.
//

#ifndef object_properties_hpp
#define object_properties_hpp

class object_properties
{
public:
    object_properties();
    virtual ~object_properties();
    
	shared_ptr<sprite_properties> sprite;
	shared_ptr<LightArea> light;

	string properName;
	string clsName;

	SpaceVect dimensions;
	SpaceFloat mass = 0.0;
	SpaceFloat friction = 0.0;

	int _refcount = 0;

	inline virtual type_index getType() const { return typeid(*this); }

	getter(shared_ptr<sprite_properties>, sprite)
	getter(shared_ptr<LightArea>, light)
	getter(string, properName)
	getter(string, clsName)
	getter(SpaceVect, dimensions)
	getter(SpaceFloat, mass)
	getter(SpaceFloat, friction)

	setter(shared_ptr<sprite_properties>, sprite)
	setter(shared_ptr<LightArea>, light)
	setter(string, properName)
	setter(string, clsName)
	setter(SpaceVect, dimensions)
	setter(SpaceFloat, mass)
	setter(SpaceFloat, friction)
};

class agent_properties : public object_properties
{
public:
	inline agent_properties() {}
	virtual inline ~agent_properties() {}

	AttributeMap attributes;

	string ai_package;

	const SpellDesc* attack;
	list<const SpellDesc*> spellInventory;
	list<effect_entry> effects;

	SpaceFloat viewRange = 0.0;
	SpaceFloat viewAngle = 0.0;

	bool detectEssence = false;
	bool isFlying = false;

	inline virtual type_index getType() const { return typeid(*this); }
};

class bomb_properties : public object_properties
{
public:
    bomb_properties();
    virtual ~bomb_properties();
    
	string explosionSound;

	float blastRadius;
	float fuseTime;
	float cost;

	DamageInfo damage;

	inline virtual type_index getType() const { return typeid(*this); }
};

class bullet_properties : public object_properties
{
public:
    bullet_properties();
    virtual ~bullet_properties();
    
	SpaceFloat speed;

	DamageInfo damage;

	int hitCount = 1;
	int ricochetCount = 0;

	bool invisible = false;
	bool ignoreObstacles = false;
	bool deflectBullets = false;

	bullet_properties clone();

	inline virtual type_index getType() const { return typeid(*this); }

	getter(SpaceFloat, speed)
	getter(DamageInfo, damage)
	getter(int, hitCount)
	getter(int, ricochetCount)
	getter(bool, invisible)
	getter(bool, ignoreObstacles)
	getter(bool, deflectBullets)

	setter(SpaceFloat, speed)
	setter(DamageInfo, damage)
	setter(int, hitCount)
	setter(int, ricochetCount)
	setter(bool, invisible)
	setter(bool, ignoreObstacles)
	setter(bool, deflectBullets)
};

class effectarea_properties : public object_properties
{
public:
    effectarea_properties();
    virtual ~effectarea_properties();
    
	const MagicEffectDescriptor* effect;
	float magnitude = 0.0f;
};

class enemy_properties : public agent_properties
{
public:
	inline enemy_properties() {}

	string firepattern;
	string collectible;

	DamageInfo touchEffect;

	inline virtual type_index getType() const { return typeid(*this); }
};

class environment_object_properties : public object_properties
{
public:
    environment_object_properties();
    virtual ~environment_object_properties();
    
	string scriptName;
	string interactionIcon;
	PhysicsLayers layers = PhysicsLayers::none;
	bool interactible;

	inline virtual type_index getType() const { return typeid(*this); }
};

class floorsegment_properties : public object_properties {
public:
    floorsegment_properties();
    virtual ~floorsegment_properties();
    
	string sfxRes;
	string sprite;

	double traction = 1.0;

	bool pressurePlate = false;

	inline virtual type_index getType() const { return typeid(*this); }
};

class item_properties : public object_properties
{
public:
	item_properties();
    virtual ~item_properties();

	string scriptName;
	string onAcquireDialog;

	bool addToInventory = true;

	inline virtual type_index getType() const { return typeid(*this); }
};

struct dialog_entry
{
	dialog_entry();
	dialog_entry(string dialog);
	dialog_entry(
		function<bool(NPC*)> condition,
		function<void(NPC*)> effect,
		string dialog,
		bool once
	);

	function<bool(NPC*)> condition;
	function<void(NPC*)> effect;

	string dialog;
	bool once = false;

	int _refcount = 0;
};

class npc_properties : public agent_properties
{
public:
	inline npc_properties() {}
	~npc_properties();

	list<local_shared_ptr<dialog_entry>> dialogs;

	inline virtual type_index getType() const { return typeid(*this); }
};

#endif /* object_properties_hpp */
