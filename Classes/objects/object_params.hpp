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

	SpaceVect dimensions;

	bool hidden = false;
};

#endif
