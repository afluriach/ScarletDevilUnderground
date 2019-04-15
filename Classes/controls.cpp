//
//  controls.cpp
//  Koumachika
//
//  Created by Toni on 10/15/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "functional.hpp"
#include "GScene.hpp"
#include "macros.h"

const bool ControlRegister::logKeyEvents = false;
const bool ControlRegister::logButtons = false;
const bool ControlRegister::logActionState = false;
const float ControlRegister::deadzone = 0.1f;
const float ControlRegister::deadzone2 = deadzone*deadzone;
const float ControlRegister::triggerDeadzone = 0.2f;

#define key_action_1(key_id,action_id) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action_id)}
#define key_action_2(key_id,action1,action2) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2)}
#define key_action_3(key_id,action1,action2,action3) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2) | make_enum_bitfield(ControlAction::action3)}

const unordered_map<EventKeyboard::KeyCode, ControlActionState> ControlRegister::keyActionMap = {
	key_action_2(KEY_ESCAPE,menuBack,pause),
	key_action_1(KEY_BACKTICK,scriptConsole),
	key_action_1(KEY_L,displayMode),
	key_action_3(KEY_ENTER,menuSelect,interact,enter),
	key_action_3(KEY_KP_ENTER, menuSelect, interact, enter),

	key_action_2(KEY_E,menuSelect,interact),
	key_action_2(KEY_X,dialogSkip, centerLook),
	key_action_1(KEY_M,mapMenu),

	key_action_1(KEY_1, spellNext),
	key_action_1(KEY_2, firePatternNext),

	key_action_1(KEY_Q, spell),
	key_action_1(KEY_C, bomb),
	key_action_1(KEY_SPACE, powerAttack),

	key_action_2(KEY_W,move_pad_up,menuUp),
	key_action_2(KEY_S, move_pad_down, menuDown),
	key_action_1(KEY_A, move_pad_left),
	key_action_1(KEY_D, move_pad_right),

	key_action_2(KEY_UP_ARROW, aim_pad_up,menuUp),
	key_action_2(KEY_DOWN_ARROW, aim_pad_down,menuDown),
	key_action_1(KEY_LEFT_ARROW, aim_pad_left),
	key_action_1(KEY_RIGHT_ARROW, aim_pad_right),

	key_action_1(KEY_Z, fireMode),

	key_action_1(KEY_LEFT_CTRL, fire),
	key_action_1(KEY_RIGHT_CTRL, fire),
	key_action_1(KEY_LEFT_ALT, sprint),
	key_action_1(KEY_RIGHT_ALT, sprint),
	key_action_1(KEY_LEFT_SHIFT, walk),
	key_action_1(KEY_RIGHT_SHIFT, walk),
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

#if use_gamepad

#define button_action_1(button_id,action_id) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action_id)}
#define button_action_2(button_id,action1,action2) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2)}
#define button_action_3(button_id,action1,action2,action3) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2) | make_enum_bitfield(ControlAction::action3)}

const unordered_map<gainput::PadButton, ControlActionState> ControlRegister::buttonActionMap = {
	button_action_1(PadButtonStart, pause),
	button_action_1(PadButtonSelect, mapMenu),

	button_action_2(PadButtonA,interact,menuSelect),
	button_action_3(PadButtonB, menuBack,dialogSkip, bomb),

	button_action_1(PadButtonL1, walk),
	button_action_1(PadButtonR1, sprint),

	button_action_1(PadButtonX, spell),
	button_action_1(PadButtonY, powerAttack),

//	button_action_1(PadButtonUp, powerAttackNext),
	button_action_1(PadButtonLeft, spellNext),
//	button_action_1(PadButtonRight, spellNext),
	button_action_1(PadButtonDown, firePatternNext),

	button_action_1(PadButtonL3, centerLook),
	button_action_1(PadButtonR3, fireMode),
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

ControlRegister::ControlRegister()
#if use_gamepad
:
input_map(manager),
gamepad_id(manager.CreateDevice<gainput::InputDevicePad>()),
gamepad(manager.GetDevice(gamepad_id))
#endif
{    
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = bindMethod(&ControlRegister::onKeyDown, this);
    keyListener->onKeyReleased = bindMethod(&ControlRegister::onKeyUp, this);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
        keyListener,
        to_int(App::EventPriorities::KeyRegisterEvent)
    );
    
    #if use_gamepad
    manager.Update();
    
    if(gamepad->IsAvailable()){
        log("Gamepad connected");
    }
    else{
        log("Gamepad not connected");
    }
    
	enum_foreach(gainput::PadButton, button_id, PadButtonStart, PadButtonMax_)
	{
		input_map.MapBool(
			button_id,
			gamepad_id,
			button_id
		);
	}
    #endif
}

ControlRegister::~ControlRegister()
{
    Director::getInstance()->getEventDispatcher()->removeEventListener(keyListener);
}

bool ControlRegister::isControlAction(ControlAction action)
{
	return bitset_enum_index(isActionPressed, action);
}

bool ControlRegister::isControlActionPressed(ControlAction action)
{
	return !bitset_enum_index(wasActionPressed, action) && bitset_enum_index(isActionPressed, action);
}

bool ControlRegister::isControlActionReleased(ControlAction action)
{
	return bitset_enum_index(wasActionPressed, action) && !bitset_enum_index(isActionPressed, action);
}

bool ControlRegister::isKeyDown(EventKeyboard::KeyCode key_id)
{
	return keysDown.find(key_id) != keysDown.end();
}

bool ControlRegister::isButtonDown(gainput::PadButton button_id)
{
	return buttonsDown.find(button_id) != buttonsDown.end();
}

SpaceVect ControlRegister::getKeyboardMovePadVector()
{
	SpaceVect result;

	bool up    = isKeyDown(EventKeyboard::KeyCode::KEY_W);
	bool down  = isKeyDown(EventKeyboard::KeyCode::KEY_S);
	bool left  = isKeyDown(EventKeyboard::KeyCode::KEY_A);
	bool right = isKeyDown(EventKeyboard::KeyCode::KEY_D);

	if (up && !down) result.y = 1;
	if (down && !up) result.y = -1;
	if (left && !right) result.x = -1;
	if (right && !left) result.x = 1;

	return result.normalizeSafe();
}

SpaceVect ControlRegister::getKeyboardAimPadVector()
{
	SpaceVect result;

	bool up    = isKeyDown(EventKeyboard::KeyCode::KEY_UP_ARROW);
	bool down  = isKeyDown(EventKeyboard::KeyCode::KEY_DOWN_ARROW);
	bool left  = isKeyDown(EventKeyboard::KeyCode::KEY_LEFT_ARROW);
	bool right = isKeyDown(EventKeyboard::KeyCode::KEY_RIGHT_ARROW);

	if (up && !down) result.y = 1;
	if (down && !up) result.y = -1;
	if (left && !right) result.x = -1;
	if (right && !left) result.x = 1;

	return result.normalizeSafe();
}

void ControlRegister::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d pressed", code);
    
	keysDown.insert(code);
}

void ControlRegister::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d released", code);
    
	keysDown.erase(code);
}

void ControlRegister::updateVectors()
{
    SpaceVect left_stick,right_stick;
    
    #if use_gamepad
    if(gamepad->IsAvailable())
    {
        left_stick.x = gamepad->GetFloat(gainput::PadButtonLeftStickX);
        left_stick.y = gamepad->GetFloat(gainput::PadButtonLeftStickY);

        right_stick.x = gamepad->GetFloat(gainput::PadButtonRightStickX);
        right_stick.y = gamepad->GetFloat(gainput::PadButtonRightStickY);

		if (gamepad->GetFloat(gainput::PadButtonAxis5) >= triggerDeadzone)
			bitset_enum_set(isActionPressed, ControlAction::fire, true);
    }
    #endif
    
    left_vector = (left_stick.length() >= deadzone) ? left_stick : getKeyboardMovePadVector();

    right_vector = (right_stick.length() >= deadzone) ? right_stick : getKeyboardAimPadVector();
    
    if(left_vector.y > 0 || right_vector.y > 0)
        bitset_enum_set(isActionPressed,ControlAction::menuUp,true);

    if(left_vector.y < 0 || right_vector.y < 0)
        bitset_enum_set(isActionPressed,ControlAction::menuDown,true);
}

ControlInfo ControlRegister::getControlInfo()
{
	ControlInfo result;

	result.action_state_crnt = isActionPressed;
	result.action_state_prev = wasActionPressed;
	result.left_v = left_vector;
	result.right_v = right_vector;

	return result;
}

ControlState ControlRegister::getControlState()
{
	ControlState result;

	result.action_state = isActionPressed;
	result.left_v = left_vector;
	result.right_v = right_vector;

	return result;
}

#define fixme
void ControlRegister::updateActionState()
{
    wasActionPressed = isActionPressed;

	isActionPressed.reset();

	for (auto it = keysDown.begin(); it != keysDown.end(); ++it) {
		auto action_it = keyActionMap.find(*it);
		if (action_it != keyActionMap.end()){
			setActions(action_it->second);
		}
	}

#if use_gamepad
	for(auto it = buttonsDown.begin(); it != buttonsDown.end(); ++it){
		auto action_it = buttonActionMap.find(*it);
		if (action_it != buttonActionMap.end()) {
			setActions(action_it->second);
		}
	}
#endif
}

void ControlRegister::setActions(ControlActionState actions_bitfield)
{
	isActionPressed |= actions_bitfield;
}

#define fixme
void ControlRegister::checkCallbacks()
{
    enum_foreach(ControlAction, action, begin, end)
    {
        if(!bitset_enum_index(wasActionPressed,action) && bitset_enum_index(isActionPressed,action)){
			if (logActionState)
				log("Action %d pressed.", action);

			auto s = onPressedID[to_size_t(action)];
			for (auto it = s.begin(); it != s.end(); ++it) {
				if (logActionState)
					log("Calling on press callback %ud.", *it);
				if(isCallbackActive.at(*it))
					onPressedCallback[*it]();
			}
        }
        
        else if(bitset_enum_index(wasActionPressed,action) && !bitset_enum_index(isActionPressed,action)){
			if (logActionState)
				log("Action %d released.", action);
			
			auto s = onReleasedID[to_size_t(action)];
			for (auto it = s.begin(); it != s.end(); ++it) {
				if (logActionState)
					log("Calling on release callback %ud.", *it);
				if (isCallbackActive.at(*it))
					onReleasedCallback[*it]();
			}
		}
    }
}

void ControlRegister::update()
{
    #if use_gamepad
    manager.Update();
    if(gamepad->IsAvailable())
        pollGamepad();
    #endif
    
    updateActionState();
    updateVectors();
    checkCallbacks();
    
    #if use_gamepad
    if(logButtons)
        logGamepadButtons();
    #endif
}

#if use_gamepad
void ControlRegister::pollGamepad()
{
	buttonsDown.clear();

	enum_foreach(gainput::PadButton,button_id, PadButtonAxisCount_,PadButtonMax_)
	{
		if (input_map.GetBool(static_cast<gainput::UserButtonId>(button_id)) ) {
			buttonsDown.insert(button_id);
		}
	}
}

void ControlRegister::logGamepadButtons()
{
    enum_foreach(gainput::PadButton, button_id, PadButtonStart, PadButtonMax_)
    {
        if(gamepad->GetBool(button_id)){
            log("Gamepad button %d pressed", button_id);
        }
    }
}
#endif

ControlRegister::callback_uuid ControlRegister::addPressListener(ControlAction action, zero_arity_function f)
{
    callback_uuid uuid = nextListenerUUID++;

    onPressedID[to_size_t(action)].insert(uuid);
    onPressedCallback[uuid] = f;
    
	isCallbackActive.insert_or_assign(uuid, true);

    return uuid;
}

ControlRegister::callback_uuid ControlRegister::addReleaseListener(ControlAction action, zero_arity_function f)
{
	callback_uuid uuid = nextListenerUUID++;

    onReleasedID[to_size_t(action)].insert(uuid);
    onReleasedCallback[uuid] = f;
    
	isCallbackActive.insert_or_assign(uuid, true);

    return uuid;
}

void ControlRegister::removeListener(callback_uuid uuid)
{
	enum_foreach(ControlAction, action, begin, end){
		onPressedID[to_size_t(action)].erase(uuid);
	}

	enum_foreach(ControlAction, action, begin, end){
		onReleasedID[to_size_t(action)].erase(uuid);
	}

    onPressedCallback.erase(uuid);
    onReleasedCallback.erase(uuid);

	isCallbackActive.erase(uuid);
}

void ControlRegister::setCallbacksActive(const list<callback_uuid>& list, bool b)
{
	for (callback_uuid id : list) {
		isCallbackActive.insert_or_assign(id, b);
	}
}

bool ControlInfo::isControlActionPressed(ControlAction id) const
{
	return action_state_crnt[to_size_t(id)] && !action_state_prev[to_size_t(id)];
}

bool ControlInfo::isControlActionReleased(ControlAction id) const
{
	return !action_state_crnt[to_size_t(id)] && action_state_prev[to_size_t(id)];
}

bool ControlInfo::isControlActionDown(ControlAction id) const
{
	return action_state_crnt[to_size_t(id)];
}

ControlState getControlState(ControlInfo info)
{
	return ControlState{info.action_state_crnt, info.left_v, info.right_v};
}

ControlListener::~ControlListener()
{
    for(ControlRegister::callback_uuid id: callback_IDs){
        App::control_register->removeListener(id);
    }
}

void ControlListener::addPressListener(ControlAction action, zero_arity_function f)
{
	ControlRegister::callback_uuid uuid = App::control_register->addPressListener(action, f);
    callback_IDs.push_back(uuid);
}
void ControlListener::addReleaseListener(ControlAction action, zero_arity_function f)
{
	ControlRegister::callback_uuid uuid = App::control_register->addReleaseListener(action, f);
    callback_IDs.push_back(uuid);
}

void ControlListener::setActive(bool b)
{
	isActive = b;
	App::control_register->setCallbacksActive(callback_IDs, b);
}
