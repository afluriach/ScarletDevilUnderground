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

class MenuLayer : public Layer
{
public:
	MenuLayer();

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
    typedef function<void(void)> listAction;

	TextListMenuLayer(
		const string& title,
		const vector<string>& options,
		const vector<listAction>& optionActions
	);

	virtual ~TextListMenuLayer();

    virtual bool init();
    
	virtual void upPressed();
	virtual void downPressed();
	virtual void selectPressed();
	virtual void backPressed();

protected:
    string title;
    vector<string> options;
    vector<listAction> optionActions;

private:
    
    //Space from top of screen to title label;
    const int titleMargin = 32;
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
    
    Node* cursor;
    size_t selected = 0;
    
	void updateCursor();
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

	inline TextListMenuImpl(const vector<string>& options, const vector<listAction>& optionActions) :
		TextListMenuLayer(
			C::title,
			options,
			optionActions
		)
	{}

};

#endif /* menu_h */
