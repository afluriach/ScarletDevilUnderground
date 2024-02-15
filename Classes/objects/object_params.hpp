//
//  object_params.hpp
//  Koumachika
//
//  Created by Toni on 6/19/19.
//
//

#ifndef object_params_hpp
#define object_params_hpp

class object_params
{
public:
	static const unordered_set<string> includedArgs;

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
	object_params(const SpaceRect& rect);
	object_params(const ValueMap& args);

	SpaceVect pos;
	SpaceFloat angle = float_pi * 0.5;
	SpaceVect vel;
	SpaceFloat angularVel = 0.0;

	SpaceVect dimensions = SpaceVect::zero;

	string name;
	map<string,string> args;

	bool hidden = false;
	bool active = false;

	getter(SpaceVect, pos);
	getter(SpaceFloat, angle);
	getter(SpaceVect, vel);
	getter(SpaceFloat, angularVel);
	getter(SpaceVect, dimensions);
	getter(string, name);
	inline const map<string,string>& get_args() const { return args; }
	getter(bool, hidden);
	getter(bool, active);

	setter(SpaceVect, pos);
	setter(SpaceFloat, angle);
	setter(SpaceVect, vel);
	setter(SpaceFloat, angularVel);
	setter(SpaceVect, dimensions);
	setter(string, name);
	setter(bool, hidden);
	setter(bool, active);
};

#define MapParams() object_params(args)

class physics_params
{
public:
	//Create circle body with fixed radius and mass
	physics_params(GType type, PhysicsLayers layers, SpaceFloat mass, bool sensor = false);
	//Create rectangle body with fixed dimensions and mass

	SpaceFloat mass;
	GType type;
	PhysicsLayers layers;

	bool sensor;
};

//The attributes of the creating agent that can affect this object. These are 
//captured from the agent when the bullet object factory is created.
struct bullet_attributes
{
	static bullet_attributes getDefault();

	parametric_space_function parametric;
	SpaceVect casterVelocity;
	gobject_ref caster;
	GType type = GType::none;
	RoomSensor* startRoom = nullptr;
	local_shared_ptr<Spell> sourceSpell;

	float size = 1.0f;
	float sizeOffset = 0.0f;
	float attackDamage = 1.0f;
	float bulletSpeed = 1.0f;
	float speedOffset = 0.0f;

	SpaceVect getDimensions(local_shared_ptr<bullet_properties> props) const;
	SpaceFloat getLaunchSpeed(local_shared_ptr<bullet_properties> props, SpaceFloat angle) const;

	getter(parametric_space_function, parametric)
	getter(SpaceVect, casterVelocity)
	getter(gobject_ref, caster)
	getter(GType, type)
	getter(RoomSensor*, startRoom)
	getter(local_shared_ptr<Spell>, sourceSpell)
	getter(float, size)
	getter(float, attackDamage)
	getter(float, bulletSpeed)
	getter(float, speedOffset)

	setter(parametric_space_function, parametric)
	setter(SpaceVect, casterVelocity)
	setter(gobject_ref, caster)
	setter(GType, type)
	setter(RoomSensor*, startRoom)
	setter(local_shared_ptr<Spell>, sourceSpell)
	setter(float, size)
	setter(float, attackDamage)
	setter(float, bulletSpeed)
	setter(float, speedOffset)
};

#endif
