//
//  replay.cpp
//  Koumachika
//
//  Created by Toni on 3/14/19.
//
//

#include "Prefix.h"

#include "replay.h"

ControlInfo Replay::getControlInfo(unsigned int frameIdx)
{
	ControlInfo result;

	if (frameIdx > 0 && frameIdx < control_data.size()) {

		result.left_v = control_data[frameIdx].left_v;
		result.right_v = control_data[frameIdx].right_v;

		result.action_state_crnt = control_data[frameIdx].action_state;
		result.action_state_prev = control_data[frameIdx - 1].action_state;
	}
	else {
		log("Replay out of bounds, frame: %d", frameIdx);
	}

	return result;
}
