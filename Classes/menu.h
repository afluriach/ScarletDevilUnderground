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

#include "cocos2d.h"

#include <boost/bind.hpp>

#include "App.h"
#include "Graphics.h"
#include "util.h"

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
        
        cocos2d::Size screenSize = getScreenSize();
        
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
        
        cursor = DiamondCursor::create();
        addChild(cursor);
        cursor->setPositionX(leftMargin/2);
        updateCursor();
        
        keyListener = new KeyListener(this);
        
        keyListener->addPressListener(Keys::up, boost::bind( &TextListMenuLayer::upPressed, this));
        keyListener->addPressListener(Keys::down, boost::bind( &TextListMenuLayer::downPressed, this));
        keyListener->addPressListener(Keys::action, boost::bind( &TextListMenuLayer::selectPressed, this));
        
        keyListener->addReleaseListener(Keys::up, boost::bind( &TextListMenuLayer::upReleased, this));
        keyListener->addReleaseListener(Keys::down, boost::bind( &TextListMenuLayer::downReleased, this));
        
        return true;
    }
    
protected:
    std::string title;
    std::vector<std::string> options;
    std::vector<std::function<void()>> optionActions;
    
    bool upHeld = false;
    bool downHeld = false;
    
    inline TextListMenuLayer(
        const std::string& title,
        const std::vector<std::string>& options,
        const std::vector<std::function<void()>>& optionActions
    ) :
    title(title),
    options(options),
    optionActions(optionActions) {}
    
    inline void upPressed()
    {
        upHeld = true;
        if(downHeld) return;
        
        --selected;
        selected %= options.size();
        updateCursor();
    }
    inline void downPressed()
    {
        downHeld = true;
        if(upHeld) return;
        
        ++selected;
        selected %= options.size();
        updateCursor();
    }
    inline void selectPressed()
    {
        optionActions[selected]();
    }
    
    inline void upReleased()
    {
        upHeld = false;
    }
    inline void downReleased()
    {
        downHeld = false;
    }
    

private:
    KeyListener* keyListener;
    
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

class TitleMenu : public TextListMenuLayer
{
public:
    CREATE_FUNC(TitleMenu);
    
protected:
    inline TitleMenu() : TextListMenuLayer(
        App::title,
        list_of_typed( ("Start")("Exit"), std::vector<std::string>),
        list_of_typed( (start)(exit), std::vector<std::function<void()>>)
    )
    {}
                                  
private:
    static inline void start()
    {
        pushScene<GameplayScene>();
    }
    
    static inline void exit()
    {
        end();
    }
};

#endif /* menu_h */
