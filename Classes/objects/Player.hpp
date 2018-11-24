//
//  Player.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Player_hpp
#define Player_hpp

#include "Agent.hpp"
#include "AIMixins.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "macros.h"

class Collectible;
class FirePattern;

class Player : virtual public GObject,
public Agent,
RegisterInit<Player>,
RegisterUpdate<Player>
{
public:    	
	enum class Attribute {
		health = 0,
		power,
		speed,
		acceleration,
		hitProtection,
		
		end,
	};

	typedef array<float, to_size_t(Player::Attribute::end)> AttributeSet;

	struct AttributeSystem
	{
		AttributeSet baseAttributes;
		AttributeSet modifiers;

		AttributeSystem();
		float getAdjustedValue(Attribute id) const;
	};
	
	static const int batModeInitialCost;
    static const int batModeCostPerSecond;
    
    static const float interactCooldownTime;
    
    static const float spellCooldownTime;
    static const float hitFlickerInterval;
    
	Player(const ValueMap& args);
    
	virtual float getMaxSpeed() const;
	virtual float getMaxAcceleration() const;

	virtual float getMaxHealth() const;
	virtual float getMaxPower() const;

	virtual AttributeSet getAttributes() = 0;
	virtual void setFirePatterns() = 0;
    
    //setting for player object sensing
	inline virtual float getRadarRadius() const { return 2.5f; }
	inline virtual GType getRadarType() const { return GType::objectSensor; }
    inline virtual float getDefaultFovAngle() const { return float_pi / 4.0f;}

    void hit();

    inline int getHealth(){
        return health;
    }
    
    inline void setHealth(int val){
        health = val;
    }
        
    inline int getPower(){
        return power;
    }
    
    inline bool consumePower(int val){
        if(power >= val){
            power -= val;
            return true;
        }
        return false;
    }
    
    inline bool isSpellProtectionMode() const {return spellProtectionMode;}
    inline void setSpellProtectionMode(bool mode) {spellProtectionMode = mode;}
    
    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 20.0f;}
    virtual inline GType getType() const {return GType::player;}
    
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
	void init();
    void update();
    void updateHitTime();
    void checkBaseControls();
    void checkBatModeControls();
    void updateSpellControls();
	void onSpellStop();

	void applyAttributeModifier(Attribute id, float val);
    
	inline FirePattern* getFirePattern() {
		if (firePatterns.empty())
			return nullptr;

		return firePatterns[crntFirePattern].get();
	}

	bool trySetFirePattern(int idx);
	bool trySetFirePatternNext();
	bool trySetFirePatternPrevious();

    void onCollectible(Collectible* coll);
protected:
	AttributeSystem attributeSystem;

    float hitProtectionCountdown = 0.0f;
    float spellCooldown = 0.0f;
    float interactCooldown = 0.0f;

	vector<unique_ptr<FirePattern>> firePatterns;
	int crntFirePattern = 0;
    
    int health;    
    int power;
    
    bool spellProtectionMode = false;
};

class FlandrePC : public Player
{
public:
	static const AttributeSet baseAttributeSet;

	FlandrePC(const ValueMap& args);

	virtual inline string imageSpritePath() const { return "sprites/flandre.png"; }
	virtual void setFirePatterns();
	virtual AttributeSet getAttributes();
};

class RumiaPC : public Player
{
public:
	static const AttributeSet baseAttributeSet;

	RumiaPC(const ValueMap& args);

	virtual inline string imageSpritePath() const { return "sprites/marisa.png"; }
	virtual void setFirePatterns();
	virtual AttributeSet getAttributes();
};

class CirnoPC : public Player
{
public:
	static const AttributeSet baseAttributeSet;

	CirnoPC(const ValueMap& args);
	
	virtual inline string imageSpritePath() const { return "sprites/cirno.png"; }
	virtual void setFirePatterns();
	virtual AttributeSet getAttributes();
};

#endif /* Player_hpp */
