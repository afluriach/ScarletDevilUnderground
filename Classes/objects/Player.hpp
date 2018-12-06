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
class ControlState;
class FirePattern;
class SpellDesc;

class Player :
virtual public Agent,
public RegisterInit<Player>,
public RegisterUpdate<Player>
{
public:
    static const float interactCooldownTime;
    
    static const float spellCooldownTime;
    static const float hitFlickerInterval;
    
	Player(const ValueMap& args);
    
	virtual void setFirePatterns() = 0;
	virtual void equipSpells() = 0;
    
    //setting for player object sensing
	inline virtual float getRadarRadius() const { return 2.5f; }
	inline virtual GType getRadarType() const { return GType::objectSensor; }
    inline virtual float getDefaultFovAngle() const { return float_pi / 4.0f;}

    virtual void hit(int damage, shared_ptr<MagicEffect> effect);
    
    inline bool isSpellProtectionMode() const {return spellProtectionMode;}
    inline void setSpellProtectionMode(bool mode) {spellProtectionMode = mode;}

	inline bool isFiringSuppressed() const { return suppressFiring; }
	inline void setFiringSuppressed(bool mode) { suppressFiring = mode; }

    virtual inline float getRadius() const {return 0.35f;}
    virtual inline float getMass() const {return 20.0f;}
    virtual inline GType getType() const {return GType::player;}
    
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
	void init();
    void update();
    void updateHitTime();

	void checkMovementControls(const ControlState& cs);
	void checkFireControls(const ControlState& cs);
	void checkItemInteraction(const ControlState& cs);
    void updateSpellControls(const ControlState& cs);
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
    float hitProtectionCountdown = 0.0f;
    float spellCooldown = 0.0f;
    float interactCooldown = 0.0f;

	vector<unique_ptr<FirePattern>> firePatterns;
	int crntFirePattern = 0;

	SpellDesc* equippedSpell = nullptr;
        
    bool spellProtectionMode = false;
	bool suppressFiring = false;
};

class FlandrePC : virtual public Player, public BaseAttributes<FlandrePC>
{
public:
	static const AttributeMap baseAttributes;

	FlandrePC(const ValueMap& args);

	virtual inline string imageSpritePath() const { return "sprites/flandre.png"; }
	virtual void setFirePatterns();
	virtual void equipSpells();
};

class RumiaPC : virtual public Player, public BaseAttributes<RumiaPC>
{
public:
	static const AttributeMap baseAttributes;

	RumiaPC(const ValueMap& args);

	virtual inline string imageSpritePath() const { return "sprites/marisa.png"; }
	virtual void setFirePatterns();
	virtual void equipSpells();
};

class CirnoPC : virtual public Player, public BaseAttributes<CirnoPC>
{
public:
	static const AttributeMap baseAttributes;

	CirnoPC(const ValueMap& args);
	
	virtual inline string imageSpritePath() const { return "sprites/cirno.png"; }
	virtual void setFirePatterns();
	virtual void equipSpells();
};

#endif /* Player_hpp */
