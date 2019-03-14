//
//  replay.h
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#ifndef replay_h
#define replay_h

#include "controls.h"
#include "GState.hpp"

class Replay
{
public:
	friend class boost::serialization::access;

	static const unsigned int version = 1;

	ControlInfo getControlInfo(unsigned int frameIdx);

	unsigned int random_seed;
	string scene_name;
	vector<ControlState> control_data;
	GState crnt_state;

	template<class Archive>
	inline void serialize(Archive& ar, const unsigned int version)
	{
		ar & random_seed;
		ar & scene_name;
		ar & control_data;
		ar & crnt_state;
	}
};

#endif /* replay_h */
