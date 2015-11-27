//
//  menu.cpp
//  FlansBasement
//
//  Created by Toni on 11/16/15.
//
//

#include "Prefix.h"

//scenes
#include "LibraryOpening.h"

const std::vector<std::string> SceneSelect::sceneTitles = list_of_typed(
    ("Play Scene")
    ("Library Opening")
    ("Back"),
    std::vector<std::string>
);

template <typename T>
SceneSelect::SceneLaunchAdapter sceneLaunchAdapter(){
    return []() -> void { app->pushScene<T>(); };
}

const std::vector<SceneSelect::SceneLaunchAdapter> SceneSelect::sceneActions = list_of_typed(
    (sceneLaunchAdapter<PlayScene>())
    (sceneLaunchAdapter<LibraryOpening>())
    (SceneSelect::back),
    std::vector<SceneSelect::SceneLaunchAdapter>
);

bool TextListMenuLayer::init()
{
    MenuLayer::init();
    
    cocos2d::Size screenSize = getScreenSize();
    
    titleLabel = Label::createWithSystemFont(title, titleFont, titleSize);
    
    titleLabel->setPosition(screenSize.width/2, screenSize.height - titleMargin);
    addChild(titleLabel);
    
    log("added title: %s", title.c_str());
    
    for(int i=0;i<options.size(); ++i)
    {
        std::string labelText = options[i];
        int yPos = screenSize.height - titleMargin - menuStartMargin - i*menuItemSpacing;
        
        Label* label = Label::createWithSystemFont(labelText, menuItemFont, menuItemSize);
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
    
    keyListener->addPressListener(Keys::moveUp, boost::bind( &TextListMenuLayer::upPressed, this));
    keyListener->addPressListener(Keys::arrowUp, boost::bind( &TextListMenuLayer::upPressed, this));
    
    keyListener->addPressListener(Keys::moveDown, boost::bind( &TextListMenuLayer::downPressed, this));
    keyListener->addPressListener(Keys::arrowDown, boost::bind( &TextListMenuLayer::downPressed, this));

    keyListener->addPressListener(Keys::action, boost::bind( &TextListMenuLayer::selectPressed, this));
    
    keyListener->addReleaseListener(Keys::moveUp, boost::bind( &TextListMenuLayer::upReleased, this));
    keyListener->addReleaseListener(Keys::arrowUp, boost::bind( &TextListMenuLayer::upReleased, this));
    
    keyListener->addReleaseListener(Keys::moveDown, boost::bind( &TextListMenuLayer::downReleased, this));
    keyListener->addReleaseListener(Keys::arrowDown, boost::bind( &TextListMenuLayer::downReleased, this));

    
    return true;
}
