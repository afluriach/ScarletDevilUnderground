//
//  menu.cpp
//  Koumachika
//
//  Created by Toni on 11/16/15.
//
//

#include "Prefix.h"

#include "controls.h"
#include "Graphics.h"
#include "macros.h"
#include "menu.h"
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
    
	if(selected > 0)
	    --selected;

    updateCursor();
}

void TextListMenuLayer::downPressed()
{
    downHeld = true;
    if(upHeld) return;
    
	if(selected < options.size() - 1)
	    ++selected;
    
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

void TextListMenuLayer::updateCursor()
{
	int yPos = getScreenSize().height - titleMargin - menuStartMargin - selected * menuItemSpacing;
	cursor->setPositionY(yPos);
}

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
    
    cursor = Node::ccCreate<DiamondCursor>();
    addChild(cursor);
    cursor->setPositionX(leftMargin/2);
    updateCursor();
    
    control_listener->addPressListener(ControlAction::menuUp, bind( &TextListMenuLayer::upPressed, this));
    control_listener->addPressListener(ControlAction::menuDown, bind( &TextListMenuLayer::downPressed, this));

    control_listener->addPressListener(ControlAction::menuSelect, bind( &TextListMenuLayer::selectPressed, this));
    
    control_listener->addReleaseListener(ControlAction::menuUp, bind( &TextListMenuLayer::upReleased, this));
    control_listener->addReleaseListener(ControlAction::menuDown, bind( &TextListMenuLayer::downReleased, this));
    
    return true;
}
