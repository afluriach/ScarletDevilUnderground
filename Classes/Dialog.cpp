//
//  Dialog.cpp
//  FlansBasement
//
//  Created by Toni on 11/14/15.
//
//

#include <vector>

#include "App.h"
#include "controls.h"
#include "util.h"

#include "Dialog.hpp"

using namespace std;
using namespace cocos2d;

const Color4F Dialog::backgroundColor = Color4F(0.5, 0.5, 0.5, 0.5);
const string Dialog::font = "Arial";

bool Dialog::init()
{
    Node::init();
    
    backgroundNode = DrawNode::create();
    drawBackground();
    addChild(backgroundNode, 1);

    setMsg("");    
    
    scheduleUpdate();
    
    return true;
}

void Dialog::drawBackground()
{
    backgroundNode->clear();
    
    Vec2 ll(-width/2, -height/2);
    Vec2 ur(width/2, height/2);
    
    backgroundNode->drawSolidRect(ll, ur, backgroundColor);
}

void Dialog::update(float dt)
{
    KeyRegister* kr = app->keyRegister;
    
    if(kr->isKeyHeld(Keys::action)){
        advanceFrame();
    }
}

void Dialog::advanceFrame()
{
    ++frameNum;
    
    if(frameNum < dialog.size()){
        runFrame();
    }
}

void Dialog::runFrame()
{
    dialog.at(frameNum)(*this);
}

void Dialog::processDialogFile(const std::string& text)
{
    vector<string> lines = splitString(text, "\n");
        
    dialog = std::vector<DialogFrame>();
    foreach(std::string line, lines)
    {
        dialog.push_back(setText(line));
    }
}
