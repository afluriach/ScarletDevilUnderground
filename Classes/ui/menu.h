//
//  menu.h
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef menu_h
#define menu_h

class ControlListener;
class SceneSelect;

#include "types.h"

class MenuLayer : public Layer
{
public:
	MenuLayer();
	virtual ~MenuLayer();

	virtual bool init();

	inline virtual void upPressed() {};
	inline virtual void downPressed() {};
	inline virtual void selectPressed() {};
	inline virtual void backPressed() {};

	void setControlsActive(bool b);
protected:
	unique_ptr<ControlListener> control_listener;
};

class TextListMenuLayer : public MenuLayer
{
public:
	typedef pair<string, zero_arity_function> entry;

	TextListMenuLayer(
		const string& title,
		const vector<entry>& entries
	);

	TextListMenuLayer(
		const string& title,
		const vector<string>& options,
		const vector<zero_arity_function>& optionActions
	);

	virtual ~TextListMenuLayer();

    virtual bool init();
    
	virtual void upPressed();
	virtual void downPressed();
	virtual void selectPressed();
	virtual void backPressed();

protected:
    string title;
	vector<entry> entries;
private:
    
    //Space from top of screen to title label;
    const int titleMargin = 48;
    const int titleSize = 48;
    //Space from title label to first item in menu.
    const int menuStartMargin = 100;
    const int menuItemSpacing = 60;
    const int menuItemSize = 32;
    //Space from left side of screen to menu item;
    const int leftMargin = 72;
    //Space between the left side of the menu item to the cursor.
    const int cursorMargin = 5;
    
    Label* titleLabel;
    vector<Label*> menuItemLabels;
    
	CCSize screenSize;
	float scale;

    Node* cursor;
    size_t selected = 0;
    
	void updateCursor();
	int yPosition(int idx);
};

template<class C>
class TextListMenuImpl : public TextListMenuLayer
{
public:
	inline TextListMenuImpl() :
	TextListMenuLayer(
		C::title,
		C::entries,
		C::entryActions
	)
	{}

	inline TextListMenuImpl(const vector<entry>& entries) :
		TextListMenuLayer(
			C::title,
			entries
		)
	{}

	inline TextListMenuImpl(const vector<string>& options, const vector<zero_arity_function>& optionActions) :
		TextListMenuLayer(
			C::title,
			options,
			optionActions
		)
	{}

	inline virtual ~TextListMenuImpl() {}
};

#endif /* menu_h */
