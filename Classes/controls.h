//
//  controls.h
//  Koumachika
//
//  Created by Toni on 10/15/15.
//
//

#ifndef controls_h
#define controls_h

enum Keys
{
    moveUp = 1,
    moveDown,
    moveLeft,
    moveRight,
    arrowUp,
    arrowDown,
    arrowLeft,
    arrowRight,
    action,
    backtick,
    num1,
    enter,
};

typedef unordered_map<EventKeyboard::KeyCode, Keys, enum_hash> KeyCodeMap;

//Map the Cocos key code to the enum used to identify it.
//This describes all of the keys that will be tracked by the register, and also all
//keys that can be handled by the KeyListener.
extern const KeyCodeMap watchedKeys;

//Tracks all key events. Used globally to poll key state.
class KeyRegister
{
public:
    bool isKeyDown(const Keys& key);
    static const bool logKeyEvents = false;
    
    //Gets the state of the move or arrow key.
    Vec2 getMoveKeyState();
    Vec2 getArrowKeyState();

private:
    friend class App;
    KeyRegister();

    void onKeyDown(EventKeyboard::KeyCode, Event*);
    void onKeyUp(EventKeyboard::KeyCode, Event*);
    
    unordered_map<Keys, bool, enum_hash> keyDown;
    
    EventListenerKeyboard* keyListener;
};

//Listens for key presses and calls the associated callback.
//This may attach to a Node's event handler or the Direcor's global one.
class KeyListener
{
public:
    KeyListener();
    KeyListener(Node* node);
    
    inline void addPressListener(Keys k, function<void()> f)
    {
        onPressed[k] = f;
    }
    
    inline void addReleaseListener(Keys k, function<void()> f)
    {
        onReleased[k] = f;
    }
    
private:
    void initListener();
    void onKeyDown(EventKeyboard::KeyCode, Event*);
    void onKeyUp(EventKeyboard::KeyCode, Event*);
    
    EventListenerKeyboard* keyListener;
    unordered_map<Keys, function<void()>, enum_hash> onPressed;
    unordered_map<Keys, function<void()>, enum_hash> onReleased;
    
};

#endif /* controls_h */
