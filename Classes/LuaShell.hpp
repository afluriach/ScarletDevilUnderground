//
//  LuaShell.hpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#ifndef LuaShell_hpp
#define LuaShell_hpp

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "App.h"

USING_NS_CC;

class LuaShell : public Layer
{
public:
    CREATE_FUNC(LuaShell);
    static constexpr int width = 700;
    static constexpr int height = 250;
    static const Color4F backgroundColor;

    virtual bool init();
    virtual void setVisible(bool visible);
    
    void runText();
protected:
    void createEditBox();

    DrawNode* background;
    ui::EditBox* editBox;    
};


#endif /* LuaShell_hpp */
