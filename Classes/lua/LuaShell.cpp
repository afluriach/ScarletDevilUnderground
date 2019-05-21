//
//  LuaShell.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "LuaAPI.hpp"
#include "LuaShell.hpp"

const Color4F LuaShell::backgroundColor = Color4F(0.5, 0.5, 0.5, 0.5);

//On OS X, EditBoxImpl (NSTextField) catches backticks and prevents it from closing.
//Click outside the box so it does not have focus.

bool LuaShell::init()
{
    Node::init();
    
    background = DrawNode::create();
    
    Vec2 ll(-width/2, -height/2);
    Vec2 ur(width/2, height/2);
    background->drawSolidRect(ll, ur, backgroundColor);
    
    background->setPosition(app::params.width/2, height/2);
    addChild(background,1);
    
    return true;
}

//Box does not obey Node visibility rules, instead must be created when the shell is shown.
void LuaShell::createEditBox()
{
    //The coordinates used by the edit box do not obey the window scaling
    editBox = ui::EditBox::create(cocos2d::CCSize(width,height),nullptr);
    editBox->setPosition(Vec2(app::params.width, height)/2);
    //Return type shouldn't matter for desktop UI.
    editBox->setInputMode(ui::EditBox::InputMode::ANY);
    editBox->setFontName("Courier");
    editBox->setFontSize(16);
    editBox->setText("--Lua Code Here");
    addChild(editBox,2);
}

void LuaShell::setVisible(bool visible)
{
    if(visible)
        createEditBox();
    else
        removeChild(editBox);
    
    Layer::setVisible(visible);
}

void LuaShell::runText()
{
    log("running script: %s", editBox->getText());
    App::lua->runString(editBox->getText());
}

string LuaShell::getText()
{
    return editBox->getText();
}
