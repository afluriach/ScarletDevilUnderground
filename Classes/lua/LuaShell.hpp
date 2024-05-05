//
//  LuaShell.hpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#ifndef LuaShell_hpp
#define LuaShell_hpp

class LuaShell : public Layer
{
public:
	static constexpr int width = 700;
	static constexpr int height = 250;
	static const Color4F backgroundColor;

	virtual bool init();
	virtual void setVisible(bool visible);
	
	void runText();
	string getText();
protected:
	void createEditBox();

	DrawNode* background;
	ui::EditBox* editBox;	
};


#endif /* LuaShell_hpp */
