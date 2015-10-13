//
//  menu.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef menu_h
#define menu_h

#include <vector>

#include <boost/assign.hpp>
#include "cocos2d.h"

#include "Graphics.h"

using namespace std::placeholders;

class MenuLayer : public cocos2d::Layer
{
    
};

class TextListMenuLayer : public MenuLayer
{
public:
    
    virtual bool init()
    {
        MenuLayer::init();
        
        Size screenSize = getScreenSize();
        
        titleLabel = cocos2d::Label::createWithSystemFont(title, titleFont, titleSize);
        
        titleLabel->setPosition(screenSize.width/2, screenSize.height - titleMargin);
        addChild(titleLabel);
        
        log("added title: %s", title.c_str());
        
        for(int i=0;i<options.size(); ++i)
        {
            std::string labelText = options[i];
            int yPos = screenSize.height - titleMargin - menuStartMargin - i*menuItemSpacing;
            
            cocos2d::Label* label = cocos2d::Label::createWithSystemFont(labelText, menuItemFont, menuItemSize);
            menuItemLabels.push_back(label);
            label->setPosition(leftMargin + label->getContentSize().width/2, yPos);
            addChild(label);
            
            log("added %s", labelText.c_str());
        }
        
        cursor = Diamond::create();
        addChild(cursor);
        cursor->setPositionX(leftMargin/2);
        updateCursor();
        
        keyListener = EventListenerKeyboard::create();
        keyListener->onKeyPressed = std::bind(&TextListMenuLayer::onKeyPressed, this, _1, _2);
        keyListener->onKeyReleased = std::bind(&TextListMenuLayer::onKeyReleased, this, _1, _2);
        getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);
        
        return true;
    }
    
protected:
    std::string title;
    std::vector<std::string> options;
    std::vector<std::function<void()>> optionActions;
    
    bool upHeld = false;
    bool downHeld = false;
    void onKeyPressed(EventKeyboard::KeyCode code, Event* event)
    {
        if(code == cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW && !upHeld)
        {
            upHeld = true;
            --selected;
            selected %= options.size();
            updateCursor();
        }
        if(code == cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW && !downHeld)
        {
            downHeld = true;
            ++selected;
            selected %= options.size();
            updateCursor();
        }
        if(code == cocos2d::EventKeyboard::KeyCode::KEY_Z)
        {
            optionActions[selected]();
        }

    }
    void onKeyReleased(EventKeyboard::KeyCode code, Event* event)
    {
        if(code == cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW)
        {
            upHeld = false;
        }
        if(code == cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW)
        {
            downHeld = false;
        }
    }

private:
    cocos2d::EventListenerKeyboard* keyListener;
    
    //Space from top of screen to title label;
    const int titleMargin = 32;
    const int titleSize = 48;
    const std::string titleFont = "Arial";
    //Space from title label to first item in menu.
    const int menuStartMargin = 100;
    const int menuItemSpacing = 60;
    const int menuItemSize = 32;
    const std::string menuItemFont = "Arial";
    //Space from left side of screen to menu item;
    const int leftMargin = 72;
    //Space between the left side of the menu item to the cursor.
    const int cursorMargin = 5;
    
    cocos2d::Label* titleLabel;
    std::vector<cocos2d::Label*> menuItemLabels;
    
    cocos2d::Node* cursor;
    int selected = 0;
    
    void updateCursor()
    {
        int yPos = getScreenSize().height - titleMargin - menuStartMargin - selected*menuItemSpacing;
        cursor->setPositionY(yPos);
    }
};

void start()
{
    Director::getInstance()->pushScene(createSceneFromLayer<GameplayScene>());
}

void exit()
{
    Director::getInstance()->end();
}

class TitleMenu : public TextListMenuLayer
{
public:
    CREATE_FUNC(TitleMenu);
    virtual bool init ()
    {
        title = "フランの地下";
        options = boost::assign::list_of("Start")("Exit").convert_to_container<std::vector<std::string>>();
        optionActions = boost::assign::list_of(start)(exit).convert_to_container<std::vector<std::function<void()>>>();
        
        TextListMenuLayer::init();

        return true;
    }
};

#endif /* menu_h */
