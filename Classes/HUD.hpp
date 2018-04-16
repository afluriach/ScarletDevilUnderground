//
//  HUD.hpp
//  Koumachika
//
//  Created by Toni on 12/7/15.
//
//

#ifndef HUD_hpp
#define HUD_hpp

class Player;

class HealthBar : public Node
{
public:
    static const int heartSize = 32;

    CREATE_FUNC(HealthBar);
    void setMax(int);
    void setValue(int);
    void runFlicker();
protected:
    Vector<Sprite*> heartSprites;
    int crntVal = 0;
};

//An icon with a numeric counter next to it.
class Counter : public Node
{
public:
    static const int iconSize = 96;
    
    //Distance between icon and counter label.
    static const int spacing = 16;

    inline Counter(const string& iconRes, const int val)
    {
        init(iconRes, val);
        autorelease();
    }
    void init(const string& iconRes, const int val);
    void setVal(const int val);
    void setIcon(const string& iconRes);
private:
    Sprite* icon;
    Label* counter;
    int val;
};

class PowerMeter : public Node
{
public:
    CREATE_FUNC(PowerMeter);

    static const int iconSize = 64;
    
    //Distance between icon and counter label.
    static const int spacing = 32;

    bool init();
    void setVal(int val);
private:
    Sprite* icon;
    Label* counter;
    int val;
};

class HUD : public Layer
{
public:
    static const int height = 50;
    
    static const int fontSize;
    
//    static const Color4F backgroundColor;

    CREATE_FUNC(HUD);
    HUD();
    //Not an override of Layer
    void update();
    virtual bool init();

    HealthBar* health;
    PowerMeter* power;
    Counter* objectiveCounter;    
    Sprite* interactionIcon;
    
    inline void showHealth(bool val)
    {
        health->setVisible(val);
    }
    
    inline void setInteractionIcon(const string& val)
    {
        interactionIcon->setVisible(!val.empty());
        interactionIcon->setTexture(val);
    }
    
protected:
    Player* player;
};

#endif /* HUD_hpp */
