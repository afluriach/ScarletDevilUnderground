//
//  controls.h
//  FlansBasement
//
//  Created by Toni on 10/15/15.
//
//

#ifndef controls_h
#define controls_h

enum Keys
{
    up = 1,
    down,
    left,
    right,
    action,
    backtick,
    enter,
};

typedef std::map<cocos2d::EventKeyboard::KeyCode, Keys> KeyCodeMap;

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
private:
    friend class App;
    KeyRegister();

    void onKeyDown(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    void onKeyUp(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    
    std::map<Keys, bool> keyDown;
    
    cocos2d::EventListenerKeyboard* keyListener;
};

//Listens for key presses and calls the associated callback.
//This may attach to a Node's event handler or the Direcor's global one.
class KeyListener
{
public:
    KeyListener();
    KeyListener(cocos2d::Node* node);
    
    inline void addPressListener(Keys k, std::function<void()> f)
    {
        onPressed[k] = f;
    }
    
    inline void addReleaseListener(Keys k, std::function<void()> f)
    {
        onReleased[k] = f;
    }
    
private:
    void initListener();
    void onKeyDown(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    void onKeyUp(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    
    cocos2d::EventListenerKeyboard* keyListener;
    std::map<Keys, std::function<void()>> onPressed;
    std::map<Keys, std::function<void()>> onReleased;
    
};

#endif /* controls_h */
