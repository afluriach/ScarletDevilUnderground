//
//  menu.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef menu_h
#define menu_h

#include "BlockScene.h"

class SceneSelect;

class MenuLayer : public GScene
{
    
};

class TextListMenuLayer : public MenuLayer
{
public:
    
    virtual bool init();
    
protected:
    string title;
    vector<string> options;
    vector<function<void()>> optionActions;
    
    bool upHeld = false;
    bool downHeld = false;
    
    TextListMenuLayer(
        const string& title,
        const vector<string>& options,
        const vector<function<void()>>& optionActions
    );
    
    void upPressed();
    void downPressed();
    void selectPressed();
    void upReleased();
    void downReleased();

private:
    KeyListener* keyListener;
    
    //Space from top of screen to title label;
    const int titleMargin = 32;
    const int titleSize = 48;
    const string titleFont = "Arial";
    //Space from title label to first item in menu.
    const int menuStartMargin = 100;
    const int menuItemSpacing = 60;
    const int menuItemSize = 32;
    const string menuItemFont = "Arial";
    //Space from left side of screen to menu item;
    const int leftMargin = 72;
    //Space between the left side of the menu item to the cursor.
    const int cursorMargin = 5;
    
    Label* titleLabel;
    vector<Label*> menuItemLabels;
    
    Node* cursor;
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
    TitleMenu();
private:
    static void start();
    static void sceneSelect();
    static void exit();
};

class SceneSelect : public TextListMenuLayer
{
public:
    typedef function<void(void)> SceneLaunchAdapter;
    
    static const vector<string> sceneTitles;
    static const vector<SceneLaunchAdapter> sceneActions;
    
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
