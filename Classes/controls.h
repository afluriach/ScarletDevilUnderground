//
//  controls.h
//  Koumachika
//
//  Created by Toni on 10/15/15.
//
//

#ifndef controls_h
#define controls_h

#include "enum.h"
#include "types.h"

enum class ControlAction
{
	begin = 1,
    pause = 1,
    scriptConsole,
	displayMode,

    menu_select,
    menu_back,
    menuUp,
    menuDown,
	map_menu,

    dialog_skip,
    interact,
    enter,

	fire,
    spell,
	bomb,
	power_attack,

	fire_pattern_next,

	spell_previous,
	spell_next,

	powerAttackNext,

	center_look,
	fire_mode,

	move_up,
	move_right,
	move_down,
	move_left,

	aim_up,
	aim_right,
	aim_down,
	aim_left,

	sprint,
	focus,

	end
};

typedef bitset<to_size_t(ControlAction::end)> ControlActionState;

class ControlInfo
{
public:
	bool isControlActionPressed(ControlAction id) const;
	bool isControlActionReleased(ControlAction id) const;
	bool isControlActionDown(ControlAction id) const;

	ControlActionState action_state_prev, action_state_crnt;
	SpaceVect left_v, right_v;
};

class ControlState
{
public:
	friend class boost::serialization::access;

	static const unsigned int version = 1;

	ControlActionState action_state;
	SpaceVect left_v, right_v;

	template<class Archive>
	inline void serialize(Archive& ar, const unsigned int version)
	{
		ar & action_state;
		ar & left_v;
		ar & right_v;
	}
};

ControlState getControlState(ControlInfo info);

//Tracks all key events. Used globally to poll key state.
class ControlRegister
{
public:
	typedef unsigned int callback_uuid;

    static const bool logKeyEvents;
    static const bool logButtons;
	static const bool logActionState;
    static const float deadzone;
	static const float deadzone2;
	static const float triggerDeadzone;
        
    static const unordered_map<EventKeyboard::KeyCode, ControlActionState> defaultKeyActionMap;
	static const unordered_map<string, EventKeyboard::KeyCode> keyNameMap;
	static const unordered_map<string, ControlAction> actionNameMap;
#if use_gamepad
    static const unordered_map<gainput::PadButton, ControlActionState> defaultButtonActionMap;
	static const unordered_map<string, gainput::PadButton> buttonNameMap;
#endif

    ControlRegister();
    ~ControlRegister();
    
    bool isControlAction(ControlAction action);
    bool isControlActionPressed(ControlAction action);
	bool isControlActionReleased(ControlAction action);

	bool isKeyDown(EventKeyboard::KeyCode key_id);
#if use_gamepad
	bool isButtonDown(gainput::PadButton button_id);
#endif
    inline SpaceVect getLeftVector(){ return left_vector;}
    inline SpaceVect getRightVector(){ return right_vector;}

	ControlState getControlState();
	ControlInfo getControlInfo();

	void applyControlSettings(const string& input);
	void clearAllKeys();
	void clearKeyAction(const string& keyName);
	void addKeyAction(const string& keyName, const string& actionName);
#if use_gamepad
	void clearAllButtons();
	void clearButtonAction(const string& buttonName);
	void addButtonAction(const string& buttonName, const string& actionName);
	inline void setSouthpaw(bool v) { southpaw = v; }
#endif
    
	callback_uuid addPressListener(ControlAction action, zero_arity_function f);
	callback_uuid addReleaseListener(ControlAction action, zero_arity_function f);
    void removeListener(callback_uuid uuid);
	void setCallbacksActive(const list<callback_uuid>& list, bool b);
    
    void update();
private:

	template<typename E>
	inline void handleControlAssignment(
		unordered_map<E,ControlActionState>& _map,
		const unordered_map<string, E>& nameMap,
		const string& line,
		const vector<string>& tokens,
		E _end,
		string typeName
	){
		E keyButton = getOrDefault(nameMap, tokens.at(1), _end);
		if (keyButton == _end) {
			log("control_mapping.txt: Unknown %s: %s", typeName.c_str(), tokens.at(1));
			return;
		}

		if (tokens.size() == 2) {
			log("control_mapping.txt: \"%s\" ControlAction(s) missing", line.c_str());
			return;
		}

		ControlActionState result;
		bool valid = true;

		for (size_t i = 2; i < tokens.size(); ++i) {
			ControlAction action = getOrDefault(actionNameMap, tokens.at(i), ControlAction::end);
			if (action == ControlAction::end) {
				valid = false;
				log("control_mapping.txt: Unknown ControlAction %s", tokens.at(i).c_str());
			}
			else {
				result |= make_enum_bitfield(action);
			}
		}

		if (valid && result.any()) {
			_map.insert_or_assign(keyButton, result);
		}
		else {
			log("control_mapping.txt: Invalid control assignment \"%s\" ignored.", line.c_str());
		}
	}

    void onKeyDown(EventKeyboard::KeyCode, Event*);
    void onKeyUp(EventKeyboard::KeyCode, Event*);
    
	SpaceVect getKeyboardMovePadVector();
	SpaceVect getKeyboardAimPadVector();

    void pollGamepad();
    void updateVectors();
    void checkCallbacks();
    void updateActionState();
    void setActions(ControlActionState actions_bitfield);
    
    void logGamepadButtons();
    
    SpaceVect left_vector, right_vector;
    
	unordered_map<EventKeyboard::KeyCode, ControlActionState> keyActionMap;
    unordered_set<EventKeyboard::KeyCode> keysDown;
#if use_gamepad
	unordered_map<gainput::PadButton, ControlActionState> buttonActionMap;
	unordered_set<gainput::PadButton> buttonsDown;
#endif

	array<unordered_set<callback_uuid>,to_size_t(ControlAction::end)> onPressedID;
	array<unordered_set<callback_uuid>, to_size_t(ControlAction::end)> onReleasedID;
    
	ControlActionState wasActionPressed;
	ControlActionState isActionPressed;
    
	unordered_map<callback_uuid, zero_arity_function> onPressedCallback;
	unordered_map<callback_uuid, zero_arity_function> onReleasedCallback;
	unordered_map<callback_uuid, bool> isCallbackActive;

    EventListenerKeyboard* keyListener;
    
    #if use_gamepad
    gainput::InputManager manager;
    gainput::InputMap input_map;
    gainput::DeviceId gamepad_id;
    gainput::InputDevice* gamepad;
    bool gamepadInitialized = false;
	bool southpaw = false;
    #endif

    callback_uuid nextListenerUUID = 1;
};

//Conveince listener class that works similarly to KeyListener. Provides automatic
//storage duration, so that an object's callbacks will be removed when it is destroyed.
class ControlListener
{
public:
    ~ControlListener();
    
    void addPressListener(ControlAction action, zero_arity_function f);
    void addReleaseListener(ControlAction action, zero_arity_function f);

	void setActive(bool b);
protected:
    list<ControlRegister::callback_uuid> callback_IDs;
	bool isActive = true;
};

#endif /* controls_h */
