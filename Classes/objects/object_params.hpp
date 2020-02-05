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
		GSpace* space,
		ObjectIDType id,
		const string& name,
		const SpaceVect& pos,
		SpaceFloat angle = float_pi * 0.5
	);
	object_params(
		const SpaceVect& pos,
		const SpaceVect& vel,
		SpaceFloat angle = float_pi * 0.5
	);
	object_params(GSpace* space, ObjectIDType id, const ValueMap& args, bool rotateUp = false);

	GSpace* space = nullptr;
	ObjectIDType id = 0;

	string name;

	SpaceVect pos;
	SpaceFloat angle = 0.0;
	SpaceVect vel;
	SpaceFloat angularVel = 0.0;

	bool hidden = false;
};

#define MapParams() make_local_shared<object_params>(space, id, args)
#define PosAngleParams(pos,angle) make_local_shared<object_params>(space, id, "", pos, angle)
#define MapParamsPointUp() PosAngleParams(getObjectPos(args), float_pi * 0.5)

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

#define MapRectPhys(type, layers, m) physics_params(type, layers, args, m)
#define MapRectPhysSensor(type, layers, m) physics_params(type, layers, args, m, true)

struct bullet_properties
{
	SpaceFloat speed;
	SpaceVect dimensions;

	DamageInfo damage;

	string sprite;
	string lightSource;

	int hitCount = 1;
	int ricochetCount = 0;

	bool directionalLaunch = true;
	bool ignoreObstacles = false;
	bool deflectBullets = false;

	bullet_properties clone();
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
};

#endif
