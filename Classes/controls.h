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
    right
};

typedef std::map<cocos2d::EventKeyboard::KeyCode, Keys> KeyCodeMap;

//Tracks all key events. Used globally to poll key state.
class KeyRegister
{
public:
    //Map the Cocos key code to the enum used to identify it
    static const KeyCodeMap watchedKeys;
    
    static inline KeyRegister* inst()
    {
        return _inst;
    }
    bool isKeyHeld(const Keys& key);
    
    static inline void init()
    {
        if(!_inst)
            _inst = new KeyRegister();
    }
private:
    KeyRegister();

    void onKeyDown(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    void onKeyUp(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    
    static KeyRegister* _inst;
    
    std::map<Keys, bool> keyHeld;
    
    cocos2d::EventListenerKeyboard* keyListener;
};

#endif /* controls_h */
