//
//  Dialog.cpp
//  Koumachika
//
//  Created by Toni on 11/14/15.
//
//

#include "Prefix.h"

#include "Dialog.hpp"
#include "FileIO.hpp"
#include "Graphics.h"
#include "GraphicsNodes.hpp"
#include "GScene.hpp"
#include "LuaAPI.hpp"

const Color4F Dialog::backgroundColor = Color4F(0.5, 0.5, 0.5, 0.5);
const Color3B Dialog::defaultTextColor = Color3B(255,255,255);

Dialog::Dialog():
control_listener(make_unique<ControlListener>())
{
}

Dialog::~Dialog()
{
    removeChild(bodyText);
    removeChild(backgroundNode);
    removeChild(cursor);
}

void Dialog::setDialog(const string& res)
{
    processDialogFile(io::loadTextFile(res));
    
    frameNum = 0;
	if (dialog.size() > 0)
		runFrame();
	else
		log("Empty dialog %s created.", res);
}

//Enabled by default, allows cursor to appear and listen for action
//button to advance frame.
void Dialog::setManualAdvance(bool manual)
{
	manualAdvance = manual;
	if (!manual)
		cursor->setVisible(false);
}

void Dialog::setAutoAdvance(bool _auto)
{
	autoAdvance = _auto;
}

void Dialog::setEndHandler(zero_arity_function f)
{
	onEnd.push_back(f);
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

	nameBackground = DrawNode::create();
	drawNameBackground();
	addChild(nameBackground, 1);
	nameBackground->setVisible(false);

	nameLabel = createTextLabel("", labelFontSize);
	nameLabel->setPosition(width / 4, height / 2 + nameLabelMargin + 0.5f*nameLabelHeight);
	nameLabel->setColor(bodyColor);
	addChild(nameLabel, 2);

    setMsg("");    
    
    cursor = Node::ccCreate<DownTriangleCursor>();
    cursor->setPosition(Vec2(width/2 - textMargin,-height/2));
    cursor->setScale(cursorScale);
    cursor->setVisible(false);
    addChild(cursor,2);
    
    scheduleUpdate();
    control_listener->addPressListener(ControlAction::menu_select, bind(&Dialog::checkManualAdvance, this));
    
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
    App::lua->runString(script);
    advanceFrame(false);
}

void Dialog::setNextScene(const string& next)
{
	onEnd.push_back(bind(&GScene::runScene, next));
}

void Dialog::unlockChamber(string id)
{
	App::crntState->registerChamberAvailable(id);
}

void Dialog::setNameLabel(string label)
{
	nameLabel->setString(label);
	nameLabel->setVisible(true);
	nameBackground->setVisible(true);
	advanceFrame(false);
}

void Dialog::clearNameLabel()
{
	nameLabel->setVisible(false);
	nameBackground->setVisible(false);
	advanceFrame(false);
}

void Dialog::drawBackground()
{
    backgroundNode->clear();
    
    Vec2 ll(-width/2, -height/2);
    Vec2 ur(width/2, height/2);
    
    backgroundNode->drawSolidRect(ll, ur, backgroundColor);
}

void Dialog::drawNameBackground()
{
	Vec2 ll(0, height / 2 + nameLabelMargin);
	Vec2 ur(width / 2, height / 2 + nameLabelMargin + nameLabelHeight);

	nameBackground->drawSolidRect(ll, ur, backgroundColor);
}

void Dialog::update(float dt)
{
    timeInFrame += dt;
    
    if(App::control_register->isControlAction(ControlAction::dialog_skip)){
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
		for(auto f : onEnd) f();
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
    for(const string& line: lines)
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
				setNextScene(tokens[1]);                
            }
			else if (boost::starts_with(line, ":unlockChamber")) {
				vector<string> tokens = splitString(line, " ");
				if (tokens.size() != 2) {
					log("invalid unlockChamber directive: %s.", line.c_str());
					continue;
				}

				dialog.push_back(makeAction<string>(
					&Dialog::unlockChamber,
					tokens[1]
				));
			}
			else if (boost::starts_with(line, ":setName ")) {
				string name = line.substr(9);

				dialog.push_back(
					makeAction<string>(&Dialog::setNameLabel, name)
				);
			}
			else if (boost::starts_with(line, ":clearName")) {
				dialog.push_back(
					makeAction(&Dialog::clearNameLabel)
				);
			}
		}
        else{
            dialog.push_back(
                makeAction<const string&>(&Dialog::setMsg,line)
            );
        }
    }
}
