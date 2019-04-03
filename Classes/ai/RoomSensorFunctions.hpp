//
//  RoomSensorFunctions.hpp
//  Koumachika
//
//  Created by Toni on 4/2/19.
//
//

#ifndef RoomSensorFunctions_hpp
#define RoomSensorFunctions_hpp

#include "AI.hpp"

namespace ai{

//Try/activate all spawners in room when room is cleared.
class SequentialClearSpawn : public Function {
public:
	SequentialClearSpawn(GSpace* space, const ValueMap& args);

	virtual void update(StateMachine& sm);

	FuncGetName(SequentialClearSpawn)
protected:
};


} //end NS

#endif /* RoomSensorFunctions_hpp */
