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
#include "macros.h"
#include "scenes.h"

const bool ControlRegister::logKeyEvents = false;
const bool ControlRegister::logButtons = false;
const bool ControlRegister::logActionState = false;
const float ControlRegister::deadzone = 0.3f;

#define key_action_1(key_id,action_id) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action_id)}
#define key_action_2(key_id,action1,action2) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2)}
#define key_action_3(key_id,action1,action2,action3) {EventKeyboard::KeyCode::key_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2) | make_enum_bitfield(ControlAction::action3)}

const map<EventKeyboard::KeyCode, ControlActionState> ControlRegister::keyActionMap = {
	key_action_2(KEY_ESCAPE,menuBack,pause),
	key_action_1(KEY_BACKTICK,scriptConsole),
	key_action_1(KEY_L,displayMode),
	key_action_3(KEY_ENTER,menuSelect,interact,enter),
	key_action_3(KEY_KP_ENTER, menuSelect, interact, enter),

	key_action_2(KEY_Z,menuSelect,interact),
	key_action_1(KEY_X,dialogSkip),
	key_action_1(KEY_M,mapMenu),

	key_action_1(KEY_1,spell1),
	key_action_1(KEY_2, spell2),
	key_action_1(KEY_3, spell3),
	key_action_1(KEY_4, spell4),

	key_action_1(KEY_COMMA, firePatternPrev),
	key_action_1(KEY_PERIOD, firePatternNext),

	key_action_2(KEY_W,move_pad_up,menuUp),
	key_action_2(KEY_S, move_pad_down, menuDown),
	key_action_1(KEY_A, move_pad_left),
	key_action_1(KEY_D, move_pad_right),

	key_action_2(KEY_UP_ARROW, aim_pad_up,menuUp),
	key_action_2(KEY_DOWN_ARROW, aim_pad_down,menuDown),
	key_action_1(KEY_LEFT_ARROW, aim_pad_left),
	key_action_1(KEY_RIGHT_ARROW, aim_pad_right),

	key_action_1(KEY_ALT, sprint),
	key_action_1(KEY_SHIFT, walk),
};

#if use_gamepad

#define button_action_1(button_id,action_id) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action_id)}
#define button_action_2(button_id,action1,action2) {gainput::PadButton::button_id, make_enum_bitfield(ControlAction::action1) | make_enum_bitfield(ControlAction::action2)}

const map<gainput::PadButton, ControlActionState> ControlRegister::buttonActionMap = {
	button_action_1(PadButtonStart, pause),
	button_action_1(PadButtonSelect, mapMenu),

	button_action_2(PadButtonA,interact,menuSelect),
	button_action_2(PadButtonB, menuBack,dialogSkip),

	button_action_1(PadButtonL1, walk),
	button_action_1(PadButtonR1, sprint),

	button_action_1(PadButtonUp, spell1),
	button_action_1(PadButtonRight, spell2),
	button_action_1(PadButtonDown, spell3),
	button_action_1(PadButtonLeft, spell4),
};
#endif

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

ControlRegister::callback_uuid ControlRegister::addPressListener(ControlAction action, function<void()> f)
{
    callback_uuid uuid = nextListenerUUID++;

    onPressedID[to_size_t(action)].insert(uuid);
    onPressedCallback[uuid] = f;
    
    return uuid;
}

ControlRegister::callback_uuid ControlRegister::addReleaseListener(ControlAction action, function<void()> f)
{
	callback_uuid uuid = nextListenerUUID++;

    onReleasedID[to_size_t(action)].insert(uuid);
    onReleasedCallback[uuid] = f;
    
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

ControlListener::~ControlListener()
{
    for(ControlRegister::callback_uuid id: callback_IDs){
        App::control_register->removeListener(id);
    }
}

void ControlListener::addPressListener(ControlAction action, function<void()> f)
{
	ControlRegister::callback_uuid uuid = App::control_register->addPressListener(action, f);
    callback_IDs.push_back(uuid);
}
void ControlListener::addReleaseListener(ControlAction action, function<void()> f)
{
	ControlRegister::callback_uuid uuid = App::control_register->addReleaseListener(action, f);
    callback_IDs.push_back(uuid);
}
