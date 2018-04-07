//
//  menu.cpp
//  Koumachika
//
//  Created by Toni on 11/16/15.
//
//

#include "Prefix.h"

//scenes
#include "LibraryOpening.h"

TextListMenuLayer::TextListMenuLayer(
    const string& title,
    const vector<string>& options,
    const vector<listAction>& optionActions
) :
title(title),
options(options),
optionActions(optionActions),
keyListener(this)
{}

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
	("Collect")
	("Stalker Room")
	("Wander")
	("Facer Floor")
    ("Back"),
    vector<string>
);

template <typename T>
TextListMenuLayer::listAction sceneLaunchAdapter(){
    return []() -> void { app->pushScene<T>(); };
}

TextListMenuLayer::listAction sceneLaunchAdapterByName(const string& name){
    return [=]() -> void { GScene::runScene(name); };
}

const vector<TextListMenuLayer::listAction> SceneSelect::sceneActions = boost::assign::list_of
	(sceneLaunchAdapterByName("BlockRoom"))
	(sceneLaunchAdapterByName("LibraryOpening"))
	(sceneLaunchAdapterByName("Collect"))
	(sceneLaunchAdapterByName("StalkerRoom"))
	(sceneLaunchAdapterByName("Wander"))
	(sceneLaunchAdapterByName("FacerFloor"))
	(SceneSelect::back)
;

bool TextListMenuLayer::init()
{
    Layer::init();
    
    cocos2d::Size screenSize = getScreenSize();
    
    titleLabel = createTextLabel(title, titleSize);
    
    titleLabel->setPosition(screenSize.width/2, screenSize.height - titleMargin);
    addChild(titleLabel);
    
    log("added title: %s", title.c_str());
    
    for(size_t i=0;i<options.size(); ++i)
    {
        string labelText = options[i];
        int yPos = screenSize.height - titleMargin - menuStartMargin - i*menuItemSpacing;
        
        Label* label = createTextLabel(labelText, menuItemSize);
        menuItemLabels.push_back(label);
        label->setPosition(leftMargin + label->getContentSize().width/2, yPos);
        addChild(label);
        
        log("added %s", labelText.c_str());
    }
    
    cursor = DiamondCursor::create();
    addChild(cursor);
    cursor->setPositionX(leftMargin/2);
    updateCursor();
    
    keyListener.addPressListener(Keys::moveUp, boost::bind( &TextListMenuLayer::upPressed, this));
    keyListener.addPressListener(Keys::arrowUp, boost::bind( &TextListMenuLayer::upPressed, this));
    
    keyListener.addPressListener(Keys::moveDown, boost::bind( &TextListMenuLayer::downPressed, this));
    keyListener.addPressListener(Keys::arrowDown, boost::bind( &TextListMenuLayer::downPressed, this));

    keyListener.addPressListener(Keys::action, boost::bind( &TextListMenuLayer::selectPressed, this));
    
    keyListener.addReleaseListener(Keys::moveUp, boost::bind( &TextListMenuLayer::upReleased, this));
    keyListener.addReleaseListener(Keys::arrowUp, boost::bind( &TextListMenuLayer::upReleased, this));
    
    keyListener.addReleaseListener(Keys::moveDown, boost::bind( &TextListMenuLayer::downReleased, this));
    keyListener.addReleaseListener(Keys::arrowDown, boost::bind( &TextListMenuLayer::downReleased, this));

    
    return true;
}

TitleMenu::TitleMenu() : TextListMenuLayer(
        App::title,
        list_of_typed( ("Start")("Scene Select")("Exit"), vector<string>),
        list_of_typed( (start)(sceneSelect)(exit), vector<listAction>)
)
{}
                                  
void TitleMenu::start()
{
    GScene::runScene("BlockRoom");
}

void TitleMenu::sceneSelect()
{
    app->pushScene<SceneSelect>();
}

void TitleMenu::exit()
{
    app->end();
}

const string PauseMenu::title = "-PAUSED-";

const vector<string> PauseMenu::entryNames = boost::assign::list_of
	("Resume")
	("Restart")
	("Exit to title")
;

const vector<TextListMenuLayer::listAction> PauseMenu::entryActions = boost::assign::list_of
	(PauseMenu::resumeScene)
	(PauseMenu::restart)
	(PauseMenu::goToTitle)
;

void PauseMenu::resumeScene()
{
	GScene::playScene()->exitPause();
}

void PauseMenu::restart()
{
	GScene::restartScene();
}

void PauseMenu::goToTitle()
{
	GScene::runScene("TitleMenu");
}
