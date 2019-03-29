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

class RadialMeterShader;
class Enemy;
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
	static const unsigned int boundingSize = 256;
	static const unsigned int radiusMargin = 32;

	RadialMeter(RadialMeterSettings settings);
	RadialMeter(string iconName, Color4F empty, Color4F filled);
	void setValue(float v);

	virtual bool init();
	virtual void setOpacity(GLubyte ch);
protected:
	void redraw();

	string iconName;
	Color4F filled, empty;

	RadialMeterShader* cone;
	Sprite* icon;

	float crntValue = 0.0f;
};

struct LinearMeterSettings
{
	Color4F fillColor;
	Color4F emptyColor;
//	string icon;
};

class LinearMeter : public Node
{
public:
	static const Vec2 boundingSize;
	static const float outlineWidth;

	LinearMeter(LinearMeterSettings settings);

	virtual bool init();

	void setValue(float newValue);
	void setMax(float maxValue);
protected:
	void redraw();

	LinearMeterSettings settings;
	DrawNode* draw;

	float crntValue = 0.0f;
	float maxValue = 0.0f;
};

class MagicEffects : public Node
{
public:
	//Radial Meters are scaled down by 1/2
	static const int spacing;
	static const float totalAutohideTime;
	static const float fadeoutTime;

	static const vector<pair<Attribute, RadialMeterSettings>> meterSettings;

	MagicEffects();

	void setPercentValue(Attribute element, int p);
	void reorganize();

	virtual bool init();
	void update();
	Map<Attribute,RadialMeter*> meters;
	unordered_map<Attribute, int> values;
	unordered_map<Attribute, float> cooldownTimers;
	unordered_map<Attribute, bool> fadeoutFlags;
};


//An icon with a numeric counter next to it.
class Counter : public Node
{
public:
    static const int iconSize = 96;
    
    //Distance between icon and counter label.
    static const int spacing = 16;

	Counter(const string& iconRes, int val, int maxVal = 0);

    virtual bool init();
    void setVal(int _val);
	void setMaxVal(int _maxVal);
    void setIcon(const string& iconRes);
private:
	string makeLabel();
	//should be called whenever the counter label text is changed
	void repositionText();

    Sprite* icon;
    Label* counter;
    int val;
	int maxVal;
	string iconRes;
};

class EnemyInfo : public Node
{
public:
	static const int hWidth;
	static const int hHeight;

	EnemyInfo();

	virtual bool init();
	void update();
	void setEnemy(string _name, float _hp, float _maxHP);
	void setEnemyHealth(float _hp);
	bool isValid();
protected:
	float hp = 0.0f, maxHP = 0.0f;
	DrawNode* healthBar;
	Label* name;
	Label* hpLabel;
};

class HUD : public Layer
{
public:
	static constexpr bool showAll = false;

	static const LinearMeterSettings hpSettings;
	static const LinearMeterSettings mpSettings;
	static const LinearMeterSettings staminaSettings;

    static const int height = 50;

    static const int fontSize;
    
//    static const Color4F backgroundColor;

	HUD(GSpace* space);
    virtual ~HUD();
    //Not an override of Layer
    void update();
    virtual bool init();

	LinearMeter* hpMeter;
	LinearMeter* mpMeter;
	LinearMeter* staminaMeter;

	Counter* keyMeter;
	Counter* mapFragmentMeter;
	Counter* objectiveCounter;

	MagicEffects* magicEffects;

    Sprite* interactionIcon;
	Sprite* firePatternIcon;
	Sprite* powerAttackIcon;
	Sprite* bombIcon;
	Sprite* spellIcon;

	EnemyInfo* enemyInfo;

	Label* performanceStats = nullptr;
	float performanceStatsTimer = 0.0f;

	void setHP(int);
	void setMaxHP(int);

	void setMP(int);
	void setMaxMP(int);

	void setStamina(int);
	void setMaxStamina(int);
	void setKeyCount(int);

	void runHealthFlicker(float length, float interval);
	void runMagicFlicker(float length, float interval);
	void runPowerFlicker(float duration);

	void setPercentValue(Attribute element, int val);

	void setObjectiveCounter(string iconRes, int val);
	void setObjectiveCounterVisible(bool val);

	void initMapCounter(int mapCount);
	void setMapCounter(int val);
    
	void setPerformanceStats();
	void updatePerformanceStats();
	void clearPerformanceStats();

	void setMansionMode(bool val);    
	void setInteractionIcon(string val);
	void setFirePatternIcon(string val);
	void setSpellIcon(string val);
	void setPowerAttackIcon(string val);

	void setEnemyInfo(string name, float hp, float maxHP);
	void updateEnemyInfo(float hp);
	void clearEnemyInfo();

protected:
	GSpace* space;
	bool isMansionMode = false;
};

#endif /* HUD_hpp */
