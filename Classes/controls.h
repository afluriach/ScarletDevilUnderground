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
#include "util.h"

enum class GamepadButton
{
    start = 1,
    a,
    b,
    dpadUp,
    dpadRight,
    dpadDown,
    dpadLeft,
};

enum class ControlAction
{
	begin = 1,
    pause = 1,
    scriptConsole,

    menuSelect,
    menuBack,
    menuUp,
    menuDown,

    dialogSkip,
    interact,
    enter,

    spell1,
    spell2,
    spell3,
    spell4,

	firePatternPrev,
	firePatternNext,

	move_pad_up,
	move_pad_right,
	move_pad_down,
	move_pad_left,

	aim_pad_up,
	aim_pad_right,
	aim_pad_down,
	aim_pad_left,

	end
};

typedef bitset<to_size_t(ControlAction::end)> ControlActionState;

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

	bool isControlActionPressed(ControlAction id) const;
};


//Tracks all key events. Used globally to poll key state.
class ControlRegister
{
public:
	typedef unsigned int callback_uuid;

    static const bool logKeyEvents;
    static const bool logButtons;
	static const bool logActionState;
    static const float deadzone;
        
    static const map<EventKeyboard::KeyCode, ControlActionState> keyActionMap;
#if use_gamepad
    static const map<gainput::PadButton, ControlActionState> buttonActionMap;
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
    
	callback_uuid addPressListener(ControlAction action, function<void()> f);
	callback_uuid addReleaseListener(ControlAction action, function<void()> f);
    void removeListener(callback_uuid uuid);
    
    void update();
private:
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
    SpaceVect prev_left_vector, prev_right_vector;
    
    set<EventKeyboard::KeyCode> keysDown;
#if use_gamepad
	set<gainput::PadButton> buttonsDown;
#endif

	array<set<callback_uuid>,to_size_t(ControlAction::end)> onPressedID;
	array<set<callback_uuid>, to_size_t(ControlAction::end)> onReleasedID;
    
	ControlActionState wasActionPressed;
	ControlActionState isActionPressed;
    
	map<callback_uuid, function<void()>> onPressedCallback;
	map<callback_uuid, function<void()>> onReleasedCallback;

    EventListenerKeyboard* keyListener;
    
    #if use_gamepad
    gainput::InputManager manager;
    gainput::InputMap input_map;
    gainput::DeviceId gamepad_id;
    gainput::InputDevice* gamepad;
    bool gamepadInitialized = false;
    #endif

    callback_uuid nextListenerUUID = 1;
};

class ControlReplay
{
public:
	friend class boost::serialization::access;

	static const unsigned int version = 1;

	bool load(const string& filepath);
	bool save(const string& filepath);

	string scene_name;
	vector<ControlState> control_data;

	template<class Archive>
	inline void serialize(Archive& ar, const unsigned int version)
	{
		ar & scene_name;
		ar & control_data;
	}
};

//Conveince listener class that works similarly to KeyListener. Provides automatic
//storage duration, so that an object's callbacks will be removed when it is destroyed.
class ControlListener
{
public:
    ~ControlListener();
    
    void addPressListener(ControlAction action, function<void()> f);
    void addReleaseListener(ControlAction action, function<void()> f);
protected:
    list<ControlRegister::callback_uuid> callback_IDs;
};

#endif /* controls_h */
