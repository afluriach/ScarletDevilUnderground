//
//  HUD.hpp
//  Koumachika
//
//  Created by Toni on 12/7/15.
//
//

#ifndef HUD_hpp
#define HUD_hpp

#include "object_ref.hpp"

class Player;

class HealthBar : public Node
{
public:
    static const int heartSize = 32;

    void setMax(int);
    void setValue(int);
    void runFlicker(float duration);
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

	Counter(const string& iconRes, const int val);

    virtual bool init();
    void setVal(const int val);
    void setIcon(const string& iconRes);
private:
    Sprite* icon;
    Label* counter;
    int val;
	string iconRes;
};

class PowerMeter : public Node
{
public:
    static const int iconSize = 64;
    
    //Distance between icon and counter label.
    static const int spacing = 32;

    bool init();
    void setVal(int val);
    void runFlicker();
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

	HUD(GSpace* space, object_ref<Player> player);
    virtual ~HUD();
    //Not an override of Layer
    void update();
    virtual bool init();

    HealthBar* health;
    PowerMeter* power;
    Counter* objectiveCounter;    
    Sprite* interactionIcon;
	Sprite* firePatternIcon;
    
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
    object_ref<Player> player;
	GSpace* space;
};

#endif /* HUD_hpp */
