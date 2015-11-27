//
//  Dialog.cpp
//  FlansBasement
//
//  Created by Toni on 11/14/15.
//
//

#include "Prefix.h"

#include "Dialog.hpp"

const Color4F Dialog::backgroundColor = Color4F(0.5, 0.5, 0.5, 0.5);
const string Dialog::font = "Arial";

void Dialog::checkAdvanceFrame()
{
    if(timeInFrame >= frameWaitTime){
        advanceFrame();
        cursor->reset();
        cursor->setVisible(false);
    }
}

bool Dialog::init()
{
    Node::init();
    
    backgroundNode = DrawNode::create();
    drawBackground();
    addChild(backgroundNode, 1);

    setMsg("");    
    
    cursor = DownTriangleCursor::create();
    cursor->setPosition(Vec2(width/2 - textMargin,-height/2));
    cursor->setScale(cursorScale);
    cursor->setVisible(false);
    addChild(cursor,2);
    
    scheduleUpdate();
    keyListener.addPressListener(Keys::action, bind(&Dialog::checkAdvanceFrame, this));
    
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
    timeInFrame += dt;
    
    if(timeInFrame > frameWaitTime)
        cursor->setVisible(true);
}

void Dialog::advanceFrame()
{
    ++frameNum;
    timeInFrame = 0;
    
    if(frameNum < dialog.size()){
        runFrame();
    }
}

void Dialog::runFrame()
{
    dialog.at(frameNum)(*this);
}

DialogFrame makeSetColor(const string& line)
{
    vector<string> tokens = splitString(line, " ");
    
    if(tokens.size() != 4){
        log("Invalid setColor directive: %s", line.c_str());
        return setColor(Color3B(255,255,255));
    }
    
    try{
        log("%s %s %s", tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str());
        return setColor(Color3B(
            boost::lexical_cast<int>(tokens[1]),
            boost::lexical_cast<int>(tokens[2]),
            boost::lexical_cast<int>(tokens[3])
        ));
    } catch(boost::bad_lexical_cast){
        log("setColor parse error: %s", line.c_str());
        return setColor(Color3B(255,255,255));
    }
}

void Dialog::processDialogFile(const string& text)
{
    vector<string> lines = splitString(text, "\n");
        
    dialog = vector<DialogFrame>();
    foreach(string line, lines)
    {
        //Check for directives
        if(boost::starts_with(line, ":")){
            if(boost::starts_with(line, ":setColor")){
                dialog.push_back(makeSetColor(line));
                continue;
            }
            else if(boost::starts_with(line, ":lua ")){
                dialog.push_back(runLua(line.substr(5)));
                continue;
            }
        }
        
        dialog.push_back(setText(line));
    }
}
