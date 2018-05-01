//
//  menu.cpp
//  Koumachika
//
//  Created by Toni on 11/16/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "LibraryOpening.h"
#include "macros.h"
#include "menu.h"
#include "PlayScene.hpp"
#include "scenes.h"

TextListMenuLayer::TextListMenuLayer(
    const string& title,
    const vector<string>& options,
    const vector<listAction>& optionActions
) :
control_listener(make_unique<ControlListener>()),
title(title),
options(options),
optionActions(optionActions)
{}

TextListMenuLayer::~TextListMenuLayer() {}

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


const vector<string> SceneSelect::sceneTitles = boost::assign::list_of
	("Collect")
	("Stalker Room")
	("Wander")
	("Facer Floor")
    ("SakuyaRoom")
    ("Library")
    ("Back")
;

template <typename T>
TextListMenuLayer::listAction sceneLaunchAdapter(){
    return []() -> void { app->runScene<T>(); };
}

TextListMenuLayer::listAction sceneLaunchAdapterByName(const string& name){
    return [=]() -> void { GScene::runScene(name); };
}

const vector<TextListMenuLayer::listAction> SceneSelect::sceneActions = boost::assign::list_of
	(sceneLaunchAdapterByName("Collect"))
	(sceneLaunchAdapterByName("StalkerRoom"))
	(sceneLaunchAdapterByName("Wander"))
	(sceneLaunchAdapterByName("FacerFloor"))
    (sceneLaunchAdapterByName("SakuyaRoom"))
    (sceneLaunchAdapterByName("Library"))
	(SceneSelect::back)
;

bool TextListMenuLayer::init()
{
    Layer::init();
    
    cocos2d::CCSize screenSize = getScreenSize();
    
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
    
    control_listener->addPressListener(ControlAction::menuUp, boost::bind( &TextListMenuLayer::upPressed, this));
    control_listener->addPressListener(ControlAction::menuDown, boost::bind( &TextListMenuLayer::downPressed, this));

    control_listener->addPressListener(ControlAction::menuSelect, boost::bind( &TextListMenuLayer::selectPressed, this));
    
    control_listener->addReleaseListener(ControlAction::menuUp, boost::bind( &TextListMenuLayer::upReleased, this));
    control_listener->addReleaseListener(ControlAction::menuDown, boost::bind( &TextListMenuLayer::downReleased, this));
    
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
    app->runScene<SceneSelect>();
}

void TitleMenu::exit()
{
    app->end();
}

void SceneSelect::back()
{
    app->runScene<TitleMenu>();
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
	GScene::crntScene->exitPause();
}

void PauseMenu::restart()
{
	GScene::restartScene();
}

void PauseMenu::goToTitle()
{
	GScene::runScene("TitleMenu");
}

const string GameOverMenu::title = "GAME OVER";

const vector<string> GameOverMenu::entryNames = boost::assign::list_of
	("Restart")
	("Exit to title")
;

const vector<TextListMenuLayer::listAction> GameOverMenu::entryActions = boost::assign::list_of
	(PauseMenu::restart)
	(PauseMenu::goToTitle)
;

void GameOverMenu::restart()
{
	GScene::restartScene();
}

void GameOverMenu::goToTitle()
{
	GScene::runScene("TitleMenu");
}

