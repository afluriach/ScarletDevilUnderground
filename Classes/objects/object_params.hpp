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
	object_params(GSpace* space, ObjectIDType id, const ValueMap& args);

	GSpace* space;
	ObjectIDType id;

	string name;

	SpaceVect pos;
	SpaceFloat angle = 0.0;
	SpaceVect vel = SpaceVect::zero;
	SpaceFloat angularVel = 0.0;

	SpaceVect dimensions = SpaceVect::zero;

	bool hidden = false;
};

#endif
