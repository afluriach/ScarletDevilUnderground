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
	#define pfunc(n) void n(StateMachine* fsm, const ValueMap& args)

	void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args);

	pfunc(red_fairy);
	pfunc(zombie_fairy);
	pfunc(patchouli_enemy);

} //end NS

#endif