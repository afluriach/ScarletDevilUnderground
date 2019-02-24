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
#include "Graphics.h"
#include "macros.h"
#include "menu.h"
#include "scenes.h"

MenuLayer::MenuLayer() :
	control_listener(make_unique<ControlListener>())
{
}

MenuLayer::~MenuLayer() {}

bool MenuLayer::init()
{
	Layer::init();

	control_listener->addPressListener(ControlAction::menuUp, bind(&MenuLayer::upPressed, this));
	control_listener->addPressListener(ControlAction::menuDown, bind(&MenuLayer::downPressed, this));

	control_listener->addPressListener(ControlAction::menuSelect, bind(&MenuLayer::selectPressed, this));
	control_listener->addPressListener(ControlAction::menuBack, bind(&MenuLayer::backPressed, this));

	return true;
}

void MenuLayer::setControlsActive(bool b)
{
	control_listener->setActive(b);
}

TextListMenuLayer::TextListMenuLayer(
    const string& title,
    const vector<string>& options,
    const vector<listAction>& optionActions
) :
title(title),
options(options),
optionActions(optionActions)
{}

TextListMenuLayer::~TextListMenuLayer() {}

void TextListMenuLayer::upPressed()
{
	if(selected > 0)
	    --selected;

    updateCursor();
}

void TextListMenuLayer::downPressed()
{
	if(selected < options.size() - 1)
	    ++selected;
    
    updateCursor();
}

void TextListMenuLayer::selectPressed()
{
    optionActions[selected]();
}

void TextListMenuLayer::backPressed()
{
	App::getCrntScene()->popMenuIfNonroot();
}

void TextListMenuLayer::updateCursor()
{
	int yPos = yPosition(selected);
	cursor->setPositionY(yPos);
}

int TextListMenuLayer::yPosition(int idx)
{
	return screenSize.height - (titleMargin + menuStartMargin + idx * menuItemSpacing)*scale;
}

bool TextListMenuLayer::init()
{
    MenuLayer::init();
    
    screenSize = getScreenSize();
	scale = App::getScale();
    
    titleLabel = createTextLabel(title, titleSize*scale);
    
    titleLabel->setPosition(screenSize.width/2, screenSize.height - titleMargin*scale);
    addChild(titleLabel);
    
    for(size_t i=0;i<options.size(); ++i)
    {
        string labelText = options[i];
        int yPos = yPosition(i);
        
        Label* label = createTextLabel(labelText, menuItemSize*scale);
        menuItemLabels.push_back(label);
        label->setPosition(leftMargin*1.5f*scale + label->getContentSize().width/2, yPos);
        addChild(label);
    }
    
    cursor = Node::ccCreate<DiamondCursor>();
	cursor->setScale(0.5f*scale);
    addChild(cursor);
    cursor->setPositionX(leftMargin*0.75f*scale);
    updateCursor();
    
    return true;
}
