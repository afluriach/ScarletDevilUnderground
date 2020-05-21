//
//  object_params.hpp
//  Koumachika
//
//  Created by Toni on 6/19/19.
//
//

#ifndef object_params_hpp
#define object_params_hpp

class RoomSensor;
class Spell;

class object_params
{
public:
	inline object_params() {}
	object_params(
		const SpaceVect& pos,
		SpaceFloat angle = float_pi * 0.5
	);
	object_params(
		const SpaceVect& pos,
		const SpaceVect& vel,
		SpaceFloat angle = float_pi * 0.5
	);
	object_params(const ValueMap& args);

	SpaceVect pos;
	SpaceFloat angle = float_pi * 0.5;
	SpaceVect vel;
	SpaceFloat angularVel = 0.0;

	string name;
	string ai_package;

	int level = 0;

	bool hidden = false;
	bool active = false;

	getter(SpaceVect, pos);
	getter(SpaceFloat, angle);
	getter(SpaceVect, vel);
	getter(SpaceFloat, angularVel);
	getter(string, name);
	getter(string, ai_package);
	getter(int, level);
	getter(bool, hidden);
	getter(bool, active);

	setter(SpaceVect, pos);
	setter(SpaceFloat, angle);
	setter(SpaceVect, vel);
	setter(SpaceFloat, angularVel);
	setter(string, name);
	setter(string, ai_package);
	setter(int, level);
	setter(bool, hidden);
	setter(bool, active);
};

#define MapParams() object_params(args)

class physics_params
{
public:
	//Create circle body with fixed radius and mass
	physics_params(GType type, PhysicsLayers layers, SpaceFloat radius, SpaceFloat mass, bool sensor = false);
	//Create rectangle body with fixed dimensions and mass
	physics_params(GType type, PhysicsLayers layers, SpaceVect dimensions, SpaceFloat mass, bool sensor = false);
	//Create rectangle body with variable dimensions and default mass
	physics_params(GType type, PhysicsLayers layers, const ValueMap& args, SpaceFloat mass, bool sensor = false);

	SpaceVect dimensions;
	SpaceFloat mass;
	GType type;
	PhysicsLayers layers;

	bool sensor;
};

class object_properties
{
public:
	shared_ptr<sprite_properties> sprite;
	shared_ptr<LightArea> light;

	string properName;
	string clsName;

	SpaceVect dimensions;
	SpaceFloat mass = 0.0;
	SpaceFloat friction = 0.0;

	inline virtual type_index getType() const { return typeid(*this); }
};

#define MapRectPhys(type, layers, m) physics_params(type, layers, args, m)
#define MapRectPhysSensor(type, layers, m) physics_params(type, layers, args, m, true)

class bullet_properties : public object_properties
{
public:
	SpaceFloat speed;

	DamageInfo damage;

	int hitCount = 1;
	int ricochetCount = 0;

	bool invisible = false;
	bool directionalLaunch = true;
	bool ignoreObstacles = false;
	bool deflectBullets = false;

	bullet_properties clone();

	inline virtual type_index getType() const { return typeid(*this); }
};

//The attributes of the creating agent that can affect this object. These are 
//captured from the agent when the bullet object factory is created.
struct bullet_attributes
{
	static bullet_attributes getDefault();

	SpaceVect casterVelocity;
	gobject_ref caster;
	GType type;
	RoomSensor* startRoom = nullptr;
	unsigned int sourceSpell = 0;

	float size = 1.0f;
	float attackDamage = 1.0f;
	float bulletSpeed = 1.0f;

	inline SpaceVect getCasterVel() const { return casterVelocity; }
	inline gobject_ref getCaster() const { return caster; }
	inline GType getType() const { return type; }
	inline RoomSensor* getStartRoom() const { return startRoom; }
	inline unsigned int getSourceSpell() const { return sourceSpell; }
	inline float getSize() const { return size; }
	inline float getAttackDamage() const { return attackDamage; }
	inline float getBulletSpeed() const { return bulletSpeed; }

	inline void setCasterVel(SpaceVect v) { casterVelocity = v; }
	inline void setCaster(gobject_ref v) { caster = v; }
	inline void setType(GType v) { type = v; }
	inline void setStartRoom(RoomSensor* v) { startRoom = v; }
	inline void setSourceSpell(unsigned int v) { sourceSpell = v; }
	inline void setSize(float v) { size = v; }
	inline void setAttackDamage(float v) { attackDamage = v; }
	inline void setBulletSpeed(float v) { bulletSpeed = v; }
};

class floorsegment_properties : public object_properties {
public:
	string sfxRes;
	string sprite;

	double traction = 1.0;

	bool pressurePlate = false;

	inline virtual type_index getType() const { return typeid(*this); }
};

#endif
