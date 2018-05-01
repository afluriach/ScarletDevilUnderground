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
#include "macros.h"
#include "scenes.h"

//Backtick "KEY_TILDE" enum does not work on OS X.
const EventKeyboard::KeyCode backtickKey = static_cast<EventKeyboard::KeyCode>(123);
//Similarly, neither KEY_RETURN nor KEY_ENTER are recognized
const EventKeyboard::KeyCode returnKey = static_cast<EventKeyboard::KeyCode>(35);

const bool ControlRegister::logKeyEvents = false;
const bool ControlRegister::logButtons = false;
const float ControlRegister::deadzone = 0.3f;

const KeyCodeMap ControlRegister::watchedKeys = boost::assign::map_list_of
    (EventKeyboard::KeyCode::KEY_Z, KeyboardKey::z)
    (EventKeyboard::KeyCode::KEY_X, KeyboardKey::x)
    (backtickKey, KeyboardKey::backtick)
    (EventKeyboard::KeyCode::KEY_1, KeyboardKey::num1)
    (EventKeyboard::KeyCode::KEY_2, KeyboardKey::num2)
    (EventKeyboard::KeyCode::KEY_3, KeyboardKey::num3)
    (EventKeyboard::KeyCode::KEY_4, KeyboardKey::num4)
    (returnKey, KeyboardKey::enter)
    (EventKeyboard::KeyCode::KEY_ESCAPE, KeyboardKey::escape)
    (EventKeyboard::KeyCode::KEY_W, KeyboardKey::w)
    (EventKeyboard::KeyCode::KEY_S, KeyboardKey::s)
    (EventKeyboard::KeyCode::KEY_A, KeyboardKey::a)
    (EventKeyboard::KeyCode::KEY_D, KeyboardKey::d)
    (EventKeyboard::KeyCode::KEY_UP_ARROW, KeyboardKey::arrowUp)
    (EventKeyboard::KeyCode::KEY_DOWN_ARROW, KeyboardKey::arrowDown)
    (EventKeyboard::KeyCode::KEY_LEFT_ARROW, KeyboardKey::arrowLeft)
    (EventKeyboard::KeyCode::KEY_RIGHT_ARROW, KeyboardKey::arrowRight)
;

#if use_gamepad
const GamepadButtonMap ControlRegister::watchedButtons = boost::assign::map_list_of
    (gainput::PadButtonStart, GamepadButton::start)
    (gainput::PadButtonA, GamepadButton::a)
    (gainput::PadButtonB, GamepadButton::b)
    (gainput::PadButtonUp, GamepadButton::dpadUp)
    (gainput::PadButtonRight, GamepadButton::dpadRight)
    (gainput::PadButtonDown, GamepadButton::dpadDown)
    (gainput::PadButtonLeft, GamepadButton::dpadLeft)
;
#endif

const KeyActionMap ControlRegister::keyActionMap = boost::assign::map_list_of
    (KeyboardKey::escape, enum_bitwise_or(ControlAction,menuBack,pause))
    (KeyboardKey::backtick, ControlAction::scriptConsole)
    (KeyboardKey::enter, enum_bitwise_or3(ControlAction,menuSelect,interact,enter))
    (KeyboardKey::z, enum_bitwise_or(ControlAction,menuSelect,interact))
    (KeyboardKey::x, ControlAction::dialogSkip)
    (KeyboardKey::num1, ControlAction::spell1)
    (KeyboardKey::num2, ControlAction::spell2)
    (KeyboardKey::num3, ControlAction::spell3)
    (KeyboardKey::num4, ControlAction::spell4)
;

const ButtonActionMap ControlRegister::buttonActionMap = boost::assign::map_list_of
    (GamepadButton::start, ControlAction::pause)
    (GamepadButton::a, enum_bitwise_or(ControlAction,interact,menuSelect))
    (GamepadButton::b, enum_bitwise_or(ControlAction,menuBack,dialogSkip))
    (GamepadButton::dpadUp, ControlAction::spell1)
    (GamepadButton::dpadRight, ControlAction::spell2)
    (GamepadButton::dpadDown, ControlAction::spell3)
    (GamepadButton::dpadLeft, ControlAction::spell4)
;

ControlRegister::ControlRegister()
#if use_gamepad
:
input_map(manager),
gamepad_id(manager.CreateDevice<gainput::InputDevicePad>()),
gamepad(manager.GetDevice(gamepad_id))
#endif
{
    //Initialize key held map by putting each key enum in it.
    for(auto it = watchedKeys.begin(); it != watchedKeys.end(); ++it){
        isKeyDown[it->second] = false;
    }
    
    #if use_gamepad
    for(auto it = watchedButtons.begin(); it != watchedButtons.end(); ++it){
        isButtonDown[it->second] = false;
    }
    #endif
    
    enum_foreach(ControlAction,a,pause,end)
    {
        onPressedID[a] = list<unsigned int>();
        onReleasedID[a] = list<unsigned int>();
    }
    
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = bindMethod(&ControlRegister::onKeyDown, this);
    keyListener->onKeyReleased = bindMethod(&ControlRegister::onKeyUp, this);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
        keyListener,
        static_cast<int>(App::EventPriorities::KeyRegisterEvent)
    );
    
    #if use_gamepad
    manager.Update();
    
    if(gamepad->IsAvailable()){
        log("Gamepad connected");
    }
    else{
        log("Gamepad not connected");
    }
    
    for(auto it = watchedButtons.begin(); it != watchedButtons.end(); ++it){
        input_map.MapBool(
            static_cast<gainput::UserButtonId>(it->second),
            gamepad_id,
            it->first
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
    auto it = isActionPressed.find(action);
    
    if(it == isActionPressed.end()){
        log("Unknown ControlAction %d", action);
        return false;
    }
    else
        return it->second;
}

bool ControlRegister::isControlActionPressed(ControlAction action)
{
    auto it_prev = wasActionPressed.find(action);
    auto it_crnt = isActionPressed.find(action);
    
    if(it_prev == wasActionPressed.end() || it_crnt == isActionPressed.end()){
        log("Unknown ControlAction %d", action);
        return false;
    }
    else
        return !it_prev->second && it_crnt->second;
}

#define MOVE_KEYS isKeyDown[KeyboardKey::w], isKeyDown[KeyboardKey::s], isKeyDown[KeyboardKey::a], isKeyDown[KeyboardKey::d]
#define ARROW_KEYS isKeyDown[KeyboardKey::arrowUp], isKeyDown[KeyboardKey::arrowDown], isKeyDown[KeyboardKey::arrowLeft], isKeyDown[KeyboardKey::arrowRight]

SpaceVect getDirectionVecFromKeyQuad(bool up, bool down, bool left, bool right)
{
    SpaceVect result;

    if(up && !down) result.y = 1;
    if(down && !up) result.y = -1;
    if(left && !right) result.x = -1;
    if(right && !left) result.x = 1;
    
    return result.normalizeSafe();
}

void ControlRegister::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d pressed", code);
    
    auto key_it = watchedKeys.find(code);
    
    if(key_it != watchedKeys.end()){
        isKeyDown[key_it->second] = true;
    }
}

void ControlRegister::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d released", code);
    
    auto key_it = watchedKeys.find(code);
    
    if(key_it != watchedKeys.end())
    {
        isKeyDown[key_it->second] = false;
    }
}

#define add_key(key,padbutton) input_map.MapBool( \
    static_cast<gainput::UserButtonId>(Keys::key), \
    gamepad_id, \
    gainput::padbutton \
); \
\
wasKeyDown[Keys::key] = false;

void ControlRegister::updateVectors()
{
    prev_left_vector = left_vector;
    prev_right_vector = right_vector;

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
    
    left_vector = (left_stick.length() >= deadzone) ? left_stick :  getDirectionVecFromKeyQuad(MOVE_KEYS);

    right_vector = (right_stick.length() >= deadzone) ? right_stick :  getDirectionVecFromKeyQuad(ARROW_KEYS);
    
    if(left_vector.y > 0 || right_vector.y > 0)
        isActionPressed[ControlAction::menuUp] = true;

    if(left_vector.y < 0 || right_vector.y < 0)
        isActionPressed[ControlAction::menuDown] = true;
}

void ControlRegister::updateActionState()
{
    wasActionPressed = isActionPressed;
    
    for(auto action_it = isActionPressed.begin(); action_it != isActionPressed.end(); ++action_it){
        action_it->second = false;
    }
    
    for(auto key_it = keyActionMap.begin(); key_it != keyActionMap.end(); ++key_it){
        if(isKeyDown[key_it->first]){
            setActions(key_it->second);
        }
    }
    
    for(auto button_it = buttonActionMap.begin(); button_it != buttonActionMap.end(); ++button_it){
        if(isButtonDown[button_it->first]){
            setActions(button_it->second);
        }
    }
}

void ControlRegister::setActions(ControlAction actions_bitfield)
{
    list<ControlAction> actions = expand_enum_bitfield(actions_bitfield, ControlAction::end);
    
    BOOST_FOREACH(ControlAction a, actions)
    {
        isActionPressed[a] = true;
    }
}

void ControlRegister::callCallbacks(ControlAction action, const ActionIDMap& id_map, const CallbackMap& callback_map)
{
    list<unsigned int> IDs = id_map.at(action);

    BOOST_FOREACH(unsigned int i, IDs){
        auto it = callback_map.find(i);
        if(it != callback_map.end())
            it->second();
    }
}

void ControlRegister::checkCallbacks()
{
    enum_foreach(ControlAction, action, pause, end)
    {
        if(!wasActionPressed[action] && isActionPressed[action]){
            callCallbacks(action,onPressedID,onPressedCallback);
        }
        
        else if(wasActionPressed[action] && !isActionPressed[action]){
            callCallbacks(action,onReleasedID,onReleasedCallback);
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
    for(auto button_it = watchedButtons.begin(); button_it != watchedButtons.end(); ++button_it){
        isButtonDown[button_it->second] = input_map.GetBool(static_cast<gainput::UserButtonId>(button_it->second));
        //isButtonDown[button_it->second] = input_map.GetBool(button_it->first);
    }
}

void ControlRegister::logGamepadButtons()
{
    enum_foreach(gainput::PadButton, button_id, PadButtonStart, PadButton31)
    {
        if(gamepad->GetBool(button_id)){
            log("Gamepad button %d pressed", button_id);
        }
    }
}
#endif

unsigned int ControlRegister::addPressListener(ControlAction action, function<void()> f)
{
    int uuid = nextListenerUUID++;

    onPressedID[action].push_back(uuid);
    onPressedCallback[uuid] = f;
    
    return uuid;
}

unsigned int ControlRegister::addReleaseListener(ControlAction action, function<void()> f)
{
    int uuid = nextListenerUUID++;

    onReleasedID[action].push_back(uuid);
    onReleasedCallback[uuid] = f;
    
    return uuid;
}

void ControlRegister::removeListener(unsigned int uuid)
{
    for(auto it = onPressedID.begin(); it != onPressedID.end(); ++it){
        it->second.remove(uuid);
    }

    for(auto it = onReleasedID.begin(); it != onReleasedID.end(); ++it){
        it->second.remove(uuid);
    }

    onPressedCallback.erase(uuid);
    onReleasedCallback.erase(uuid);
}

ControlListener::~ControlListener()
{
    BOOST_FOREACH(unsigned int id, callback_IDs){
        app->control_register->removeListener(id);
    }
}

void ControlListener::addPressListener(ControlAction action, function<void()> f)
{
    unsigned int uuid = app->control_register->addPressListener(action, f);
    callback_IDs.push_back(uuid);
}
void ControlListener::addReleaseListener(ControlAction action, function<void()> f)
{
    unsigned int uuid = app->control_register->addReleaseListener(action, f);
    callback_IDs.push_back(uuid);
}
