//
//  controls.h
//  Koumachika
//
//  Created by Toni on 10/15/15.
//
//

#ifndef controls_h
#define controls_h

#include "types.h"
#include "util.h"

enum class KeyboardKey
{
    w = 1,
    s,
    a,
    d,
    arrowUp,
    arrowDown,
    arrowLeft,
    arrowRight,
    z,
    backtick,
    num1,
    enter,
	escape,
};

enum class GamepadButton
{
    start = 1,
    a,
    b,
};

enum class ControlAction
{
    pause = 1,
    scriptConsole = 2,
    menuSelect = 4,
    menuBack = 8,
    menuUp = 16,
    menuDown = 32,
    dialogSkip = 64,
    interact = 128,
    enter = 256,
    end = 512,
};

typedef unordered_map<EventKeyboard::KeyCode, KeyboardKey, enum_hash> KeyCodeMap;
typedef unordered_map<gainput::UserButtonId, GamepadButton> GamepadButtonMap;

typedef unordered_map<KeyboardKey, ControlAction, enum_hash> KeyActionMap;
typedef unordered_map<GamepadButton, ControlAction, enum_hash> ButtonActionMap;

typedef unordered_map<unsigned int, function<void()>> CallbackMap;
typedef unordered_map<ControlAction, list<unsigned int>> ActionIDMap;

//Tracks all key events. Used globally to poll key state.
class ControlRegister
{
public:
    static const bool logKeyEvents;
    static const bool logButtons;
    static const float deadzone;
    
    static const KeyCodeMap watchedKeys;
    static const GamepadButtonMap watchedButtons;
    
    static const KeyActionMap keyActionMap;
    static const ButtonActionMap buttonActionMap;
    
    ControlRegister();
    ~ControlRegister();
    
    bool isControlAction(ControlAction action);
    
    inline SpaceVect getLeftVector(){ return left_vector;}
    inline SpaceVect getRightVector(){ return right_vector;}
    
    unsigned int addPressListener(ControlAction action, function<void()> f);
    unsigned int addReleaseListener(ControlAction action, function<void()> f);
    void removeListener(unsigned int uuid);
    
    void update();
private:
    void onKeyDown(EventKeyboard::KeyCode, Event*);
    void onKeyUp(EventKeyboard::KeyCode, Event*);
    
    void pollGamepad();
    void updateVectors();
    void checkCallbacks();
    void updateActionState();
    void setActions(ControlAction actions_bitfield);
    void callCallbacks(ControlAction action, const ActionIDMap& id_Map, const CallbackMap& callback_map);
    
    void logGamepadButtons();
    
    SpaceVect left_vector, right_vector;
    SpaceVect prev_left_vector, prev_right_vector;
    
    unordered_map<KeyboardKey, bool, enum_hash> isKeyDown;
    unordered_map<GamepadButton, bool, enum_hash> isButtonDown;
    
    ActionIDMap onPressedID;
    ActionIDMap onReleasedID;
    
    unordered_map<ControlAction,bool> wasActionPressed;
    unordered_map<ControlAction,bool> isActionPressed;
    
    CallbackMap onPressedCallback;
    CallbackMap onReleasedCallback;

    EventListenerKeyboard* keyListener;
    
    gainput::InputManager manager;
    gainput::InputMap input_map;
    gainput::DeviceId gamepad_id;
    gainput::InputDevice* gamepad;
    bool gamepadInitialized = false;

    unsigned int nextListenerUUID = 1;
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
    list<unsigned int> callback_IDs;
};

#endif /* controls_h */
