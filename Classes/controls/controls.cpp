//
//  controls.cpp
//  Koumachika
//
//  Created by Toni on 10/15/15.
//
//

#include "Prefix.h"

#include "ControlAssignment.h"
//#include "controls.h"
#include "functional.hpp"
#include "GScene.hpp"

const bool ControlRegister::logKeyEvents = false;
const bool ControlRegister::logButtons = false;
const bool ControlRegister::logActionState = false;
const float ControlRegister::deadzone = 0.1f;
const float ControlRegister::deadzone2 = deadzone*deadzone;
const float ControlRegister::triggerDeadzone = 0.2f;

ControlRegister::ControlRegister()
#if use_gamepad
:
input_map(manager),
gamepad_id(manager.CreateDevice<gainput::InputDevicePad>()),
gamepad(manager.GetDevice(gamepad_id))
#endif
{    
	keyActionMap = defaultKeyActionMap;
#if use_gamepad
	buttonActionMap = defaultButtonActionMap;
#endif

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

#if use_gamepad
bool ControlRegister::isButtonDown(gainput::PadButton button_id)
{
	return buttonsDown.find(button_id) != buttonsDown.end();
}
#endif

SpaceVect ControlRegister::getKeyboardMovePadVector()
{
	SpaceVect result;

	bool up    = isControlAction(ControlAction::move_up);
	bool down  = isControlAction(ControlAction::move_down);
	bool left  = isControlAction(ControlAction::move_left);
	bool right = isControlAction(ControlAction::move_right);

	if (up && !down) result.y = 1;
	if (down && !up) result.y = -1;
	if (left && !right) result.x = -1;
	if (right && !left) result.x = 1;

	return result.normalizeSafe();
}

SpaceVect ControlRegister::getKeyboardAimPadVector()
{
	SpaceVect result;

	bool up    = isControlAction(ControlAction::aim_up);
	bool down  = isControlAction(ControlAction::aim_down);
	bool left  = isControlAction(ControlAction::aim_left);
	bool right = isControlAction(ControlAction::aim_right);

	if (up && !down) result.y = 1;
	if (down && !up) result.y = -1;
	if (left && !right) result.x = -1;
	if (right && !left) result.x = 1;

	return result.normalizeSafe();
}

void ControlRegister::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d pressed", to_int(code));
    
	keysDown.insert(code);
}

void ControlRegister::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d released", to_int(code));
    
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

    if (southpaw) swap(left_stick, right_stick);
    
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

void ControlRegister::applyControlSettings(const string& input)
{
	vector<string> lines = splitString(input, "\r\n");

	for (const string& line : lines)
	{
		vector<string> tokens = splitString(line, " \t");

		if (tokens.empty() || isComment(tokens.at(0)))
			continue;
		string front = tokens.at(0);

		if (boost::iequals(front, "clear") && tokens.size() == 3) {
			if (boost::iequals(tokens.at(1), "key")) {
				clearKeyAction(tokens.at(2));
			}
#if use_gamepad
			else if (boost::iequals(tokens.at(1), "button")) {
				clearButtonAction(tokens.at(2));
			}
#endif
		}
		else if (boost::iequals(front, "key") && tokens.size() > 2) {
			assignKey(tokens);
		}
#if use_gamepad
		else if (boost::iequals(front, "button") && tokens.size() > 2) {
			assignButton(tokens);
		}
		else if (boost::iequals(front, "southpaw") && tokens.size() >= 2) {
			southpaw = boost::lexical_cast<bool>(tokens.at(1));
		}
#endif
		else if (boost::iequals(front, "clear_all_keys")) {
			clearAllKeys();
		}
#if use_gamepad
		else if (boost::iequals(front, "clear_all_buttons")) {
			clearAllButtons();
		}
#endif
		else {
			log("control_mapping.txt: invalid line %s", line.c_str());
		}
	}
}

#if use_gamepad
void ControlRegister::assignButton(const vector<string>& v)
{
	string line = boost::join(v, " ");

	handleControlAssignment<gainput::PadButton>(
		buttonActionMap,
		buttonNameMap,
		line,
		v,
		gainput::PadButton::PadButtonMax_,
		"gamepad button"
	);
}
#endif

void ControlRegister::assignKey(const vector<string>& v)
{
	string line = boost::join(v, " ");

	handleControlAssignment<EventKeyboard::KeyCode>(
		keyActionMap,
		keyNameMap,
		line,
		v,
		EventKeyboard::KeyCode::end,
		"keycode"
	);
}

void ControlRegister::clearAllKeys()
{
	keyActionMap.clear();
}

void ControlRegister::clearKeyAction(const string& keyName)
{
	EventKeyboard::KeyCode code = getOrDefaultUpperCase(keyNameMap, keyName, EventKeyboard::KeyCode::end);

	if (code != EventKeyboard::KeyCode::end) {
		keyActionMap.erase(code);
	}
}

void ControlRegister::addKeyAction(const string& keyName, const string& actionName)
{
	EventKeyboard::KeyCode code = getOrDefaultUpperCase(keyNameMap, keyName, EventKeyboard::KeyCode::end);
	ControlAction action = getOrDefaultLowerCase(actionNameMap, actionName, ControlAction::end);

	if (code != EventKeyboard::KeyCode::end && action != ControlAction::end) {
		emplaceIfEmpty(keyActionMap, code, ControlActionState());
		keyActionMap.at(code) |= make_enum_bitfield(action);
	}
}

#if use_gamepad
void ControlRegister::clearAllButtons()
{
	buttonActionMap.clear();
}

void ControlRegister::clearButtonAction(const string& buttonName)
{
	gainput::PadButton button = getOrDefaultUpperCase(buttonNameMap, buttonName, gainput::PadButton::PadButtonMax_);

	if (button != gainput::PadButton::PadButtonMax_) {
		buttonActionMap.erase(button);
	}
}

void ControlRegister::addButtonAction(const string& buttonName, const string& actionName)
{
	gainput::PadButton button = getOrDefaultUpperCase(buttonNameMap, buttonName, gainput::PadButton::PadButtonMax_);
	ControlAction action = getOrDefaultLowerCase(actionNameMap, actionName, ControlAction::end);

	if (button != gainput::PadButton::PadButtonMax_ && action != ControlAction::end) {
		emplaceIfEmpty(buttonActionMap, button, ControlActionState());
		buttonActionMap.at(button) |= make_enum_bitfield(action);
	}
}
#endif

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
				log("Action %d pressed.", to_int(action));

			auto s = onPressedID[to_size_t(action)];
			for (auto it = s.begin(); it != s.end(); ++it) {
				if (logActionState)
					log("Calling on press callback %ud.", to_uint(*it));
				if(isCallbackActive.at(*it))
					onPressedCallback[*it]();
			}
        }
        
        else if(bitset_enum_index(wasActionPressed,action) && !bitset_enum_index(isActionPressed,action)){
			if (logActionState)
				log("Action %d released.", to_int(action));
			
			auto s = onReleasedID[to_size_t(action)];
			for (auto it = s.begin(); it != s.end(); ++it) {
				if (logActionState)
					log("Calling on release callback %ud.", to_uint(*it));
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
            log("Gamepad button %d pressed", to_int(button_id));
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
