//
//  Dialog.hpp
//  Koumachika
//
//  Created by Toni on 11/14/15.
//
//

#ifndef Dialog_hpp
#define Dialog_hpp

//A dialog frame is simply a function that takes a reference to a Dialog and applies some operation to it.
//In most cases it will set the message text.
//It can also change the title or dialog colors.

class Cursor;
class Dialog;
class ControlListener;

typedef function<void(Dialog&)> DialogFrame;

class Dialog : public Node
{
public:
    Dialog();

    virtual bool init();
    virtual void update(float dt);
    
    virtual ~Dialog();
    
    //dimensions are in UI coordinates (based on an 800x500 area)
    static const int width = 600;
    static const int height = 200;
    static const int titleSize = 30;
    static const int bodySize = 24;
	static const int labelFontSize = 18;
    static const int textMargin = 24;
	static const int nameLabelMargin = 72;
	static const int nameLabelHeight = 48;
    //The minimum time a frame must be displayed
    constexpr static float frameWaitTime = 1.2f;
    constexpr static float frameSkipTime = 0.15f;
    constexpr static float cursorScale = 0.6f;
    
    void setDialog(const string& res);
    
    //Enabled by default, allows cursor to appear and listen for action
    //button to advance frame.
	void setManualAdvance(bool manual);
	void setAutoAdvance(bool _auto);
	void setEndHandler(zero_arity_function f);
    
    template<typename... Args>
    inline DialogFrame makeAction(void (Dialog::*method)(Args...), Args... args)
    {
        return [=](Dialog& d) -> void {(d.*method)(args...);};
    }
    
    void setMsg(const string& msg);
    //After applying a directive frame that does not change the content,
    //advance to the next frame.
    void setColor(const Color3B& color);
    
    void runLuaScript(const string& script);
    void setNextScene(const string& next);
	void unlockChamber(string id);
	void setNameLabel(string label);
	void clearNameLabel();
private:
    void drawBackground();
	void drawNameBackground();
    void drawContents();
    void update();
    void runFrame();
    void advanceFrame(bool resetCursor);
    void checkTimedAdvance();
    void checkManualAdvance();
    void checkSkipAdvance();
    void processDialogFile(const string& text);
    
    static Color3B parseColorFromDirective(const string& line);
    
    static const Color4F backgroundColor;
    static const Color3B defaultTextColor;
    
    Label* bodyText;
	Label* nameLabel;
    DrawNode* backgroundNode;
	DrawNode* nameBackground;
    Cursor* cursor;
    
    unique_ptr<ControlListener> control_listener;
    
//Current state of the dialog.
    vector<DialogFrame> dialog;
    size_t frameNum = 0;
    float timeInFrame = 0;
    
    string title;
    string msg;
    Color3B bodyColor = defaultTextColor;
    
//Dialog type
    bool manualAdvance = true;
    bool autoAdvance = false;
//Events
    vector<zero_arity_function> onEnd;
};

#endif /* Dialog_hpp */
