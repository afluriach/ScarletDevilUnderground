//
//  AIPackages.hpp
//  Koumachika
//
//  Created by Toni on 6/22/19.
//
//

#ifndef AIPackages_hpp
#define AIPackages_hpp

namespace ai {
	class Function;
	class StateMachine;

	void maintain_distance(StateMachine* fsm, const ValueMap& args);
	void circle_and_fire(StateMachine* fsm, const ValueMap& args);
	void circle_around_point(StateMachine* fsm, const ValueMap& args);
	void flock(StateMachine* fsm, const ValueMap& args);

	void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args);

	void flee_player(StateMachine* fsm, const ValueMap& args);
	void idle(StateMachine* fsm, const ValueMap& args);
	void wander(StateMachine* fsm, const ValueMap& args);

} //end NS

#endif