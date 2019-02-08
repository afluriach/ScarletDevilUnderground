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
#include "controls.h"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "macros.h"
#include "object_ref.hpp"
#include "scenes.h"

class Collectible;
class ControlState;
class Door;
class EnemyBullet;
class FirePattern;
class SpellDesc;
class PlayScene;
class Upgrade;

class Player :
virtual public Agent,
public RegisterUpdate<Player>
{
public:
    static const float interactCooldownTime;
    static const float hitFlickerInterval;

	static const SpaceFloat sprintSpeedRatio;
	static const SpaceFloat focusSpeedRatio;
	
	static const SpaceFloat interactDistance;
	static const SpaceFloat grazeRadius;

	MapObjCons(Player);
	inline virtual ~Player() {}
    
	virtual void onPitfall();

	virtual void setFirePatterns() = 0;
	virtual void equipSpells() = 0;
	virtual CircleLightArea getLight() = 0;
	SpaceFloat getSpellLength();

	virtual void initializeGraphics();
    
    //setting for player object sensing
	inline virtual SpaceFloat getRadarRadius() const { return grazeRadius; }
	inline virtual GType getRadarType() const { return GType::playerGrazeRadar; }
    inline virtual SpaceFloat getDefaultFovAngle() const { return 0.0;}

	bool isProtected() const;
	void setProtection();
	void setTimedProtection(SpaceFloat seconds);
	void resetProtection();

    virtual void hit(AttributeMap attributeEffect, shared_ptr<MagicEffect> effect);

	inline bool isFiringSuppressed() const { return suppressFiring; }
	inline void setFiringSuppressed(bool mode) { suppressFiring = mode; }

	inline bool isMovementSuppressed() const { return suppressMovement; }
	inline void setMovementSuppressed(bool mode) { suppressMovement = mode; }

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	virtual inline SpaceFloat getMass() const { return 20.0; }
    virtual inline GType getType() const {return GType::player;}
    
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
	SpaceVect getInteractFeeler() const;

	void init();
    void update();
    void updateHitTime();
	void updateCombo();

	virtual void onZeroHP();

	void checkMovementControls(const ControlInfo& cs);
	void checkFireControls(const ControlInfo& cs);
	void checkItemInteraction(const ControlInfo& cs);
    void updateSpellControls(const ControlInfo& cs);
	void onSpellStop();

	SpaceFloat getSpeedMultiplier();
	void setFocusMode(bool b);
	void setSprintMode(bool b);

	unsigned int getKeyCount() const;
	void useKey();

	virtual AttributeMap getAttributeUpgrades() const;
	void applyAttributeModifier(Attribute id, float val);
    
	FirePattern* getFirePattern();

	bool trySetFirePattern(size_t idx);
	bool trySetFirePatternNext();
	bool trySetFirePatternPrevious();

    void onCollectible(Collectible* coll);
	void useDoor(Door* interactTarget);
	void moveToDestinationDoor(Door* dest);
	void applyUpgrade(Upgrade* up);

	float getComboMultiplier();
	void applyCombo(int b);

	//The bullet's graze "radar" has collided with Player.
	void onGrazeTouch(object_ref<EnemyBullet> bullet);
	//Effect is applied after the graze "radar" loses contact.
	void onGrazeCleared(object_ref<EnemyBullet> bullet);
	void invalidateGraze(object_ref<EnemyBullet> bullet);
protected:
	void applyGraze(int p);
	void startRespawn();
	void applyRespawn();

	void setHudEffect(Attribute id, Attribute max_id);
	void setHudEffect(Attribute id, float maxVal);
	void updateHudAttribute(Attribute id);

	LightID light = 0;

	SpaceVect respawnPos;
	SpaceFloat respawnAngle;
	SpaceFloat respawnTimer = 0.0;
	SpaceFloat respawnMaskTimer = 0.0;

	set<object_ref<EnemyBullet>> grazeContacts;

	float interactCooldown = 0.0f;

	vector<shared_ptr<FirePattern>> firePatterns;
	size_t crntFirePattern = 0;

	SpellDesc* equippedSpell = nullptr;

	PlayScene* playScene = nullptr;

	bool suppressFiring = false;
	bool suppressMovement = false;
	bool isSprintActive = false;
	bool isFocusActive = false;
	bool isRespawnActive = false;
	bool isComboActive = false;
};

class FlandrePC : virtual public Player, public BaseAttributes<FlandrePC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(FlandrePC);

	virtual inline string imageSpritePath() const { return "sprites/flandre.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual CircleLightArea getLight();
	virtual void setFirePatterns();
	virtual void equipSpells();
};

class RumiaPC : virtual public Player, public BaseAttributes<RumiaPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(RumiaPC);

	virtual inline string imageSpritePath() const { return "sprites/rumia.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual CircleLightArea getLight();
	virtual void setFirePatterns();
	virtual void equipSpells();
};

class CirnoPC : virtual public Player, public BaseAttributes<CirnoPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(CirnoPC);
	
	virtual inline string imageSpritePath() const { return "sprites/cirno.png"; }
	virtual CircleLightArea getLight();
	virtual void setFirePatterns();
	virtual void equipSpells();
};

#endif /* Player_hpp */
