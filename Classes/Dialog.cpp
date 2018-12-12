//
//  Dialog.cpp
//  Koumachika
//
//  Created by Toni on 11/14/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "Dialog.hpp"
#include "macros.h"
#include "scenes.h"

const Color4F Dialog::backgroundColor = Color4F(0.5, 0.5, 0.5, 0.5);
const Color3B Dialog::defaultTextColor = Color3B(255,255,255);

Dialog::Dialog():
control_listener(make_unique<ControlListener>())
{
    app->dialog = this;
}

Dialog::~Dialog()
{
    removeChild(bodyText);
    removeChild(backgroundNode);
    removeChild(cursor);
}

void Dialog::setDialog(const string& res)
{
    processDialogFile(loadTextFile(res));
    
    frameNum = 0;
    runFrame();
}


//This will advance the dialog based on time.
void Dialog::checkTimedAdvance()
{
    if(autoAdvance && timeInFrame >= frameWaitTime){
        advanceFrame(true);
    }
}

//This will advance the dialog based on interaction.
void Dialog::checkManualAdvance()
{
    if(manualAdvance && timeInFrame >= frameWaitTime){
        advanceFrame(true);
    }
}

void Dialog::checkSkipAdvance()
{
    if(timeInFrame >= frameSkipTime){
        advanceFrame(true);
    }
}

bool Dialog::init()
{
    Node::init();
    
    backgroundNode = DrawNode::create();
    drawBackground();
    addChild(backgroundNode, 1);

    setMsg("");    
    
    cursor = Node::ccCreate<DownTriangleCursor>();
    cursor->setPosition(Vec2(width/2 - textMargin,-height/2));
    cursor->setScale(cursorScale);
    cursor->setVisible(false);
    addChild(cursor,2);
    
    scheduleUpdate();
    control_listener->addPressListener(ControlAction::menuSelect, bind(&Dialog::checkManualAdvance, this));
    
    return true;
}

void Dialog::setMsg(const string& msg)
{
    this->msg = msg;
    removeChild(bodyText);
    bodyText = createTextLabel(msg, bodySize);
    bodyText->setWidth(width-textMargin*2);
    bodyText->setColor(bodyColor);
    addChild(bodyText, 2);
}

void Dialog::setColor(const Color3B& color)
{
    bodyColor = color;
    log("color set to %d %d %d", color.r, color.g, color.b);
    advanceFrame(false);
}

void Dialog::runLuaScript(const string& script)
{
    app->lua.runString(script);
    advanceFrame(false);
}

void Dialog::setNextScene(const string& next)
{
    nextScene = next;
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
    
    if(app->control_register->isControlAction(ControlAction::dialogSkip)){
        checkSkipAdvance();
    }
        
    if(timeInFrame > frameWaitTime){
        if(manualAdvance)
            cursor->setVisible(true);
        if(autoAdvance)
            checkTimedAdvance();
    }
}

void Dialog::advanceFrame(bool resetCursor)
{
    ++frameNum;
    timeInFrame = 0;
    
    if(frameNum < dialog.size()){
        runFrame();
        //Cursor should not be reset if the previous frame was a directive.
        if(resetCursor){
            cursor->reset();
            cursor->setVisible(false);
        }
    }
    else{
        if(onEnd)
            onEnd();
        if(!nextScene.empty())
            GScene::runScene(nextScene);
    }
}

void Dialog::runFrame()
{
    dialog.at(frameNum)(*this);
}

//This only includes the numerics, not the directive token.
Color3B Dialog::parseColorFromDirective(const string& line){
    vector<string> tokens = splitString(line, " ");
    
    if(tokens.size() != 4){
        log("Invalid setColor directive: %s", line.c_str());
        return defaultTextColor;
    }
    
    try{
        log("%s %s %s", tokens[1].c_str(), tokens[2].c_str(), tokens[3].c_str());
        return Color3B(
            boost::lexical_cast<int>(tokens[1]),
            boost::lexical_cast<int>(tokens[2]),
            boost::lexical_cast<int>(tokens[3])
        );
    } catch(boost::bad_lexical_cast){
        log("setColor parse error: %s", line.c_str());
        return defaultTextColor;
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
                Color3B color = parseColorFromDirective(line);
                dialog.push_back(
                    makeAction<const Color3B&>(&Dialog::setColor, color)
                );
            }
            else if(boost::starts_with(line, ":lua ")){
                dialog.push_back(
                    makeAction<const string&>(&Dialog::runLuaScript, line.substr(5))
                );
            }
            else if(boost::starts_with(line, ":nextScene")){
                vector<string> tokens = splitString(line, " ");
                if(tokens.size() != 2){
                    log("invalid nextScene directive: %s.", line.c_str());
                    continue;
                }
                if(!nextScene.empty())
                    log("Warning, old nextScene overridden: %s.", nextScene.c_str());
                nextScene = tokens[1];
                
            }
        }
        else{
            dialog.push_back(
                makeAction<const string&>(&Dialog::setMsg,line)
            );
        }
    }
}
