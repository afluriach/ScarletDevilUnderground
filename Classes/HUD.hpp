//
//  HUD.hpp
//  Koumachika
//
//  Created by Toni on 12/7/15.
//
//

#ifndef HUD_hpp
#define HUD_hpp

#include "Attributes.hpp"
#include "object_ref.hpp"

class Player;

class IconMeter : public Node
{
public:
	IconMeter(int iconSize, const string& filledIcon, const string& emptyIcon, int val, int maxVal);

	virtual bool init();

	void setMax(int);
	void setValue(int);
	void setElementalValue(float val);

	void runFlicker(float duration, float interval);
protected:
	Vector<Sprite*> iconSprites;
	int crntVal = 0;
	int maxVal = 0;
	int iconSize = 0;
	string filledIcon, emptyIcon;
};

class HealthBar : public IconMeter
{
public:
    static const int heartSize = 32;
	HealthBar();
};

class MagicBar : public IconMeter
{
public:
	static const int iconSize = 32;
	MagicBar();
};

class IceDamageBar : public IconMeter
{
public:
	static const int iconSize = 32;
	IceDamageBar();
};

class SunDamageBar : public IconMeter
{
public:
	static const int iconSize = 32;
	SunDamageBar();
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
	MagicBar* magic;
    PowerMeter* power;

	IceDamageBar* iceDamage;
	SunDamageBar* sunDamage;

	Counter* objectiveCounter;    
    Sprite* interactionIcon;
	Sprite* firePatternIcon;

	void setObjectiveCounter(string iconRes, int val);
	void setObjectiveCounterVisible(bool val);
    
	void showHealth(bool val);    
	void setInteractionIcon(const string& val);
    
protected:
    object_ref<Player> player;
	GSpace* space;
};

#endif /* HUD_hpp */
