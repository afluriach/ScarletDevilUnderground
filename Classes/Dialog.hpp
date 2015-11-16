//
//  Dialog.hpp
//  FlansBasement
//
//  Created by Toni on 11/14/15.
//
//

#ifndef Dialog_hpp
#define Dialog_hpp

#include <vector>

#include "cocos2d.h"

//A dialog frame is simply a function that takes a reference to a Dialog and applies some operation to it.
//In most cases it will set the message text.
//It can also change the title or dialog colors.
class Dialog;
typedef std::function<void(Dialog&)> DialogFrame;

class Dialog : public cocos2d::Node
{
public:
    CREATE_FUNC(Dialog);
    virtual bool init();
    virtual void update(float dt);
    
    //dimensions are in UI coordinates (based on an 800x500 area)
    static const int width = 600;
    static const int height = 200;
    static const int titleSize = 30;
    static const int bodySize = 24;
    static const int textMargin = 24;
    
    inline void setDialog(const std::vector<DialogFrame>& dialog)
    {
        this->dialog = &dialog;
        frameNum = 0;
        runFrame();
    }
    
    inline void setMsg(const std::string& msg)
    {
        this->msg = msg;
        removeChild(bodyText);
        bodyText = cocos2d::Label::createWithSystemFont(msg, font, bodySize);
        bodyText->setWidth(width-textMargin*2);
        addChild(bodyText, 2);
    }
    inline void setColor(const cocos2d::Color3B& color)
    {
        bodyText->setColor(color);
    }
private:
    void drawBackground();
    void drawContents();
    void update();
    void runFrame();
    void advanceFrame();
    
    static const cocos2d::Color4F backgroundColor;
    static const std::string font;
    
    cocos2d::Label* bodyText;
    cocos2d::DrawNode* backgroundNode;
    
//Current state of the dialog.
    const std::vector<DialogFrame>* dialog;
    int frameNum = 0;
    
    std::string title;
    std::string msg;
};

inline DialogFrame setText(const std::string& msg)
{
    return [=](Dialog& d) -> void {d.setMsg(msg);};
}

//inline DialogFrame setColor(const cocos2d::Color3B& color)
//{
//    return [=](Dialog& d) -> void {d.setColor(color);};
//}

#endif /* Dialog_hpp */
