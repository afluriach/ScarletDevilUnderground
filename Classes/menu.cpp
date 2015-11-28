//
//  menu.cpp
//  FlansBasement
//
//  Created by Toni on 11/16/15.
//
//

#include "Prefix.h"

//scenes
#include "BlockScene.h"
#include "LibraryOpening.h"

TextListMenuLayer::TextListMenuLayer(
    const string& title,
    const vector<string>& options,
    const vector<function<void()>>& optionActions
) :
title(title),
options(options),
optionActions(optionActions) {}

void TextListMenuLayer::upPressed()
{
    upHeld = true;
    if(downHeld) return;
    
    --selected;
    if(selected < 0)
        selected += options.size();

    updateCursor();
}

void TextListMenuLayer::downPressed()
{
    downHeld = true;
    if(upHeld) return;
    
    ++selected;
    if(selected >= options.size())
        selected = 0;
    
    updateCursor();
}
void TextListMenuLayer::selectPressed()
{
    optionActions[selected]();
}

void TextListMenuLayer::upReleased()
{
    upHeld = false;
}
void TextListMenuLayer::downReleased()
{
    downHeld = false;
}


const vector<string> SceneSelect::sceneTitles = list_of_typed(
    ("Block Scene")
    ("Library Opening")
    ("Back"),
    vector<string>
);

template <typename T>
SceneSelect::SceneLaunchAdapter sceneLaunchAdapter(){
    return []() -> void { app->pushScene<T>(); };
}

const vector<SceneSelect::SceneLaunchAdapter> SceneSelect::sceneActions = list_of_typed(
    (sceneLaunchAdapter<BlockScene>())
    (sceneLaunchAdapter<LibraryOpening>())
    (SceneSelect::back),
    vector<SceneSelect::SceneLaunchAdapter>
);

bool TextListMenuLayer::init()
{
    MenuLayer::init();
    
    cocos2d::Size screenSize = getScreenSize();
    
    titleLabel = Label::createWithSystemFont(title, titleFont, titleSize);
    
    titleLabel->setPosition(screenSize.width/2, screenSize.height - titleMargin);
    getLayer(sceneLayers::uiLayer)->addChild(titleLabel);
    
    log("added title: %s", title.c_str());
    
    for(int i=0;i<options.size(); ++i)
    {
        string labelText = options[i];
        int yPos = screenSize.height - titleMargin - menuStartMargin - i*menuItemSpacing;
        
        Label* label = Label::createWithSystemFont(labelText, menuItemFont, menuItemSize);
        menuItemLabels.push_back(label);
        label->setPosition(leftMargin + label->getContentSize().width/2, yPos);
        getLayer(sceneLayers::uiLayer)->addChild(label);
        
        log("added %s", labelText.c_str());
    }
    
    cursor = DiamondCursor::create();
    getLayer(sceneLayers::uiLayer)->addChild(cursor);
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

TitleMenu::TitleMenu() : TextListMenuLayer(
        App::title,
        list_of_typed( ("Start")("Scene Select")("Exit"), vector<string>),
        list_of_typed( (start)(sceneSelect)(exit), vector<function<void()>>)
)
{}
                                  
void TitleMenu::start()
{
    app->pushScene<BlockScene>();
}

void TitleMenu::sceneSelect()
{
    app->pushScene<SceneSelect>();
}

void TitleMenu::exit()
{
    app->end();
}

