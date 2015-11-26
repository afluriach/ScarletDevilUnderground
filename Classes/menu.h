//
//  menu.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef menu_h
#define menu_h

#include "PlayScene.hpp"

using namespace std::placeholders;

class SceneSelect;

class MenuLayer : public GScene
{
    
};

class TextListMenuLayer : public MenuLayer
{
public:
    
    virtual bool init();
    
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
        if(selected < 0)
            selected += options.size();

        updateCursor();
    }
    inline void downPressed()
    {
        downHeld = true;
        if(upHeld) return;
        
        ++selected;
        if(selected >= options.size())
            selected = 0;
        
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
        list_of_typed( ("Start")("Scene Select")("Exit"), std::vector<std::string>),
        list_of_typed( (start)(sceneSelect)(exit), std::vector<std::function<void()>>)
    )
    {}
                                  
private:
    static inline void start()
    {
        app->pushScene<PlayScene>();
    }
    
    static inline void sceneSelect()
    {
        app->pushScene<SceneSelect>();
    }
    
    static inline void exit()
    {
        app->end();
    }
};

class SceneSelect : public TextListMenuLayer
{
public:
    typedef std::function<void(void)> SceneLaunchAdapter;
    
    static const std::vector<std::string> sceneTitles;
    static const std::vector<SceneLaunchAdapter> sceneActions;
    
    CREATE_FUNC(SceneSelect);
    
    inline SceneSelect() :
    TextListMenuLayer(
        "Scene Select",
        sceneTitles,
        sceneActions
    )
    {}
    
protected:    
    static inline void back(){
        app->popScene();
    }
};

#endif /* menu_h */
