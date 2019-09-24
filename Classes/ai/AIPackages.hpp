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

	void maintain_distance(StateMachine* fsm, const ValueMap& args);
	void circle_and_fire(StateMachine* fsm, const ValueMap& args);
	void circle_around_point(StateMachine* fsm, const ValueMap& args);

	void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args);

	void engage_player_in_room(StateMachine* fsm, const ValueMap& args);
	void seek_player(StateMachine* fsm, const ValueMap& args);
	void flee_player(StateMachine* fsm, const ValueMap& args);

	pfunc(wander_and_flee_player);
	pfunc(bat);
	pfunc(facer);
	pfunc(follower);
	pfunc(ghost_fairy);
	pfunc(red_fairy);
	pfunc(green_fairy1);
	pfunc(green_fairy2);
	pfunc(zombie_fairy);
	pfunc(fairy2);
	pfunc(ice_fairy);
	pfunc(ghost_fairy_npc);
	pfunc(collect_marisa);
	pfunc(forest_marisa);
	pfunc(patchouli_enemy);
	pfunc(reimu_enemy);
	pfunc(rumia1);
	pfunc(rumia2);
	pfunc(sakuya);
	pfunc(sakuya_npc);
	pfunc(scorpion1);
	pfunc(scorpion2);
	pfunc(stalker);
	pfunc(evade_player_projectiles);

} //end NS

#endif