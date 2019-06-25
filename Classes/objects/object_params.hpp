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
	object_params(GSpace* space, ObjectIDType id, const ValueMap& args);

	GSpace* space = nullptr;
	ObjectIDType id = 0;

	string name;

	SpaceVect pos;
	SpaceFloat angle = 0.0;
	SpaceVect vel;
	SpaceFloat angularVel = 0.0;

	bool hidden = false;
};

#define MapParams() make_shared<object_params>(space, id, args)
#define PosAngleParams(pos,angle) make_shared<object_params>(space, id, "", pos, angle)
#define MapParamsPointUp() PosAngleParams(getObjectPos(args), float_pi * 0.5)

class physics_params
{
public:
	//Create circle body with fixed radius and mass
	physics_params(SpaceFloat radius, SpaceFloat mass);
	//Create rectangle body with fixed dimensions and mass
	physics_params(SpaceVect dimensions, SpaceFloat mass);
	//Create rectangle body with variable dimensions and default mass
	physics_params(const ValueMap& args, SpaceFloat mass);

	SpaceVect dimensions;
	SpaceFloat mass;
};

#define MapRectPhys(m) physics_params(args, mass)

#endif
