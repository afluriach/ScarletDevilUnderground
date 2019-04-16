//
//  ControlMapping.cpp
//  Koumachika
//
//  Created by Toni on 4/16/19.
//
//

#include "Prefix.h"

#include "controls.h"

#define key_action_1(key_id,action_id) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action_id)}
#define key_action_2(key_id,action1,action2) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2)}
#define key_action_3(key_id,action1,action2,action3) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2) | make_enum_bitfield(ControlAction::action3)}

const unordered_map<EventKeyboard::KeyCode, ControlActionState> ControlRegister::defaultKeyActionMap = {
	key_action_2(KEY_ESCAPE,menu_back,pause),
	key_action_1(KEY_BACKTICK,scriptConsole),
	key_action_1(KEY_L,displayMode),
	key_action_3(KEY_ENTER,menu_select,interact,enter),
	key_action_3(KEY_KP_ENTER, menu_select, interact, enter),

	key_action_2(KEY_E,menu_select,interact),
	key_action_2(KEY_X,dialog_skip, center_look),
	key_action_1(KEY_M,map_menu),

	key_action_1(KEY_1, spell_next),
	key_action_1(KEY_2, fire_pattern_next),

	key_action_1(KEY_Q, spell),
	key_action_1(KEY_C, bomb),
	key_action_1(KEY_SPACE, power_attack),

	key_action_2(KEY_W,move_up,menuUp),
	key_action_2(KEY_S, move_down, menuDown),
	key_action_1(KEY_A, move_left),
	key_action_1(KEY_D, move_right),

	key_action_2(KEY_UP_ARROW, aim_up,menuUp),
	key_action_2(KEY_DOWN_ARROW, aim_down,menuDown),
	key_action_1(KEY_LEFT_ARROW, aim_left),
	key_action_1(KEY_RIGHT_ARROW, aim_right),

	key_action_1(KEY_Z, fire_mode),

	key_action_1(KEY_LEFT_CTRL, fire),
	key_action_1(KEY_RIGHT_CTRL, fire),
	key_action_1(KEY_LEFT_ALT, sprint),
	key_action_1(KEY_RIGHT_ALT, sprint),
	key_action_1(KEY_LEFT_SHIFT, focus),
	key_action_1(KEY_RIGHT_SHIFT, focus),
};

#define entry(x) { #x, EventKeyboard::KeyCode::KEY_ ## x}

const unordered_map<string, EventKeyboard::KeyCode> ControlRegister::keyNameMap = {
	entry(ESCAPE),
	entry(BACKSPACE),
	entry(TAB),
	entry(RETURN),
	entry(CAPS_LOCK),
	entry(SHIFT),
	entry(LEFT_SHIFT),
	entry(RIGHT_SHIFT),
	entry(CTRL),
	entry(LEFT_CTRL),
	entry(RIGHT_CTRL),
	entry(ALT),
	entry(LEFT_ALT),
	entry(RIGHT_ALT),
	entry(HOME),
	entry(PG_UP),
	entry(END),
	entry(PG_DOWN),
	entry(LEFT_ARROW),
	entry(RIGHT_ARROW),
	entry(UP_ARROW),
	entry(DOWN_ARROW),
	entry(F1),
	entry(F2),
	entry(F3),
	entry(F4),
	entry(F5),
	entry(F6),
	entry(F7),
	entry(F8),
	entry(F9),
	entry(F10),
	entry(F11),
	entry(F12),
	entry(SPACE),
	entry(APOSTROPHE),
	entry(COMMA),
	entry(MINUS),
	entry(PERIOD),
	entry(SLASH),
	entry(0),
	entry(1),
	entry(2),
	entry(3),
	entry(4),
	entry(5),
	entry(6),
	entry(7),
	entry(8),
	entry(9),
	entry(SEMICOLON),
	entry(EQUAL),
	entry(LEFT_BRACKET),
	entry(BACK_SLASH),
	entry(RIGHT_BRACKET),
	entry(BACKTICK),
	entry(A),
	entry(B),
	entry(C),
	entry(D),
	entry(E),
	entry(F),
	entry(G),
	entry(H),
	entry(I),
	entry(J),
	entry(K),
	entry(L),
	entry(M),
	entry(N),
	entry(O),
	entry(P),
	entry(Q),
	entry(R),
	entry(S),
	entry(T),
	entry(U),
	entry(V),
	entry(W),
	entry(X),
	entry(Y),
	entry(Z),
	entry(ENTER),
};

#undef entry

#define entry(x) { #x, ControlAction::x }

const unordered_map<string, ControlAction> ControlRegister::actionNameMap = {
	entry(pause),
	entry(scriptConsole),
	entry(displayMode),

	entry(menu_select),
	entry(menu_back),
	entry(map_menu),

	entry(dialog_skip),
	entry(interact),

	entry(fire),
	entry(spell),
	entry(bomb),
	entry(power_attack),

	entry(fire_pattern_next),

	entry(spell_previous),
	entry(spell_next),

	entry(powerAttackNext),

	entry(center_look),
	entry(fire_mode),

	entry(move_up),
	entry(move_right),
	entry(move_down),
	entry(move_left),

	entry(aim_up),
	entry(aim_right),
	entry(aim_down),
	entry(aim_left),

	entry(sprint),
	entry(focus),
};

#undef entry

#if use_gamepad

#define button_action_1(button_id,action_id) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action_id)}
#define button_action_2(button_id,action1,action2) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2)}
#define button_action_3(button_id,action1,action2,action3) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2) | make_enum_bitfield(ControlAction::action3)}

const unordered_map<gainput::PadButton, ControlActionState> ControlRegister::defaultButtonActionMap = {
	button_action_1(PadButtonStart, pause),
	button_action_1(PadButtonSelect, map_menu),

	button_action_2(PadButtonA,interact,menu_select),
	button_action_3(PadButtonB, menu_back,dialog_skip, bomb),

	button_action_1(PadButtonL1, sprint),
	button_action_1(PadButtonR1, sprint),

	button_action_1(PadButtonL2, focus),
	button_action_1(PadButtonR2, fire),

	button_action_1(PadButtonX, spell),
	button_action_1(PadButtonY, power_attack),

//	button_action_1(PadButtonUp, powerAttackNext),
	button_action_1(PadButtonLeft, spell_next),
//	button_action_1(PadButtonRight, spell_next),
	button_action_1(PadButtonDown, fire_pattern_next),

	button_action_1(PadButtonL3, center_look),
	button_action_1(PadButtonR3, fire_mode),
};

#define entry(x) { #x, gainput::PadButton::PadButton ## x }

const unordered_map <string, gainput::PadButton > ControlRegister::buttonNameMap = {
	entry(Start),
	entry(Select),
	entry(Left),
	entry(Right),
	entry(Up),
	entry(Down),
	entry(A),
	entry(B),
	entry(X),
	entry(Y),
	entry(L1),
	entry(R1),
	entry(L2),
	entry(R2),
	entry(L3),
	entry(R3),
};

#undef entry
#endif //use_gamepad
