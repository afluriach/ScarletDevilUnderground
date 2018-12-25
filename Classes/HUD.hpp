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

typedef tuple<string, Color4F, Color4F> RadialMeterSettings;

class RadialMeter : public Node
{
public:
	static const unsigned int boundingSize = 128;
	static const unsigned int radiusMargin = 16;
	static const int segments = 128;
	//static const int iconSize = 96;

	RadialMeter(RadialMeterSettings settings);
	RadialMeter(string iconName, Color4F empty, Color4F filled);
	void setValue(float v);

	virtual bool init();
protected:
	void redraw();

	string iconName;
	Color4F filled, empty;

	DrawNode* drawNode;
	Sprite* icon;

	float crntValue = 0.0f;
};

class MagicEffects : public Node
{
public:
	//Radial Meters are scaled down by 1/2
	static const int spacing;

	static const map<Attribute, RadialMeterSettings> meterSettings;

	MagicEffects();

	void setElementalDamage(Attribute element, int val);
	void reorganize();

	virtual bool init();
	Map<Attribute,RadialMeter*> meters;
	map<Attribute, int> values;
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

	MagicEffects* magicEffects;

	Counter* objectiveCounter;    
    Sprite* interactionIcon;
	Sprite* firePatternIcon;

	void setHP(int);
	void setMaxHP(int);

	void setMP(int);
	void setMaxMP(int);

	void runHealthFlicker(float length, float interval);
	void runPowerFlicker();

	void setElementalDamage(Attribute element, int val);

	void setObjectiveCounter(string iconRes, int val);
	void setObjectiveCounterVisible(bool val);
    
	void showHealth(bool val);    
	void setInteractionIcon(string val);
	void setFirePatternIcon(string val);
    
protected:
    object_ref<Player> player;
	GSpace* space;
};

#endif /* HUD_hpp */
