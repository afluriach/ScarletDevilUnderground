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

    menuSelect,
    menuBack,
    menuUp,
    menuDown,
	mapMenu,

    dialogSkip,
    interact,
    enter,

    spell,
	bomb,
	powerAttack,

	firePatternNext,

	spellPrev,
	spellNext,

	powerAttackNext,

	move_pad_up,
	move_pad_right,
	move_pad_down,
	move_pad_left,

	aim_pad_up,
	aim_pad_right,
	aim_pad_down,
	aim_pad_left,

	sprint,
	walk,

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
        
    static const unordered_map<EventKeyboard::KeyCode, ControlActionState> keyActionMap;
#if use_gamepad
    static const unordered_map<gainput::PadButton, ControlActionState> buttonActionMap;
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
    
	callback_uuid addPressListener(ControlAction action, zero_arity_function f);
	callback_uuid addReleaseListener(ControlAction action, zero_arity_function f);
    void removeListener(callback_uuid uuid);
	void setCallbacksActive(const list<callback_uuid>& list, bool b);
    
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
    
    unordered_set<EventKeyboard::KeyCode> keysDown;
#if use_gamepad
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
