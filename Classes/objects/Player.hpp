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
#include "macros.h"
#include "object_ref.hpp"

class Collectible;
struct ControlInfo;
class ControlState;
class Door;
class EnemyBullet;
class FirePattern;
class SpellDesc;
class PlayScene;
class Upgrade;

class Player : virtual public Agent, public RadialLightObject
{
public:
	static const float centerLookHoldThresh;
    static const float interactCooldownTime;
	static const float bombCooldownTime;
    static const float hitFlickerInterval;

	static const SpaceFloat sprintSpeedRatio;
	static const SpaceFloat focusSpeedRatio;
	static const SpaceFloat bombThrowSpeed;
	
	static const SpaceFloat interactDistance;
	static const SpaceFloat grazeRadius;

	static const float bombCost;

	Player(GSpace* space, ObjectIDType id, const SpaceVect& pos, Direction d);
	MapObjCons(Player);
	inline virtual ~Player() {}
    
	virtual void onPitfall();

	void equipFirePatterns();
	void equipSpells();
	SpaceFloat getSpellLength();
	void equipPowerAttacks();

    //setting for player object sensing
	inline virtual SpaceFloat getRadarRadius() const { return grazeRadius; }
	inline virtual GType getRadarType() const { return GType::playerGrazeRadar; }
    inline virtual SpaceFloat getDefaultFovAngle() const { return 0.0;}

	void setProtection();
	void setTimedProtection(SpaceFloat seconds);
	void resetProtection();

	virtual void onBulletCollide(Bullet* b);
	virtual void onBulletHitTarget(Bullet* bullet, Agent* target);
    virtual bool hit(DamageInfo damage);

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
    virtual void update();
	void updateCombo();

	virtual void onZeroHP();

	void checkMovementControls(const ControlInfo& cs);
	void checkFireControls(const ControlInfo& cs);
	void checkBombControls(const ControlInfo& cs);
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
    
    void onCollectible(Collectible* coll);
	void useDoor(Door* interactTarget);
	void moveToDestinationDoor(Door* dest);
	void applyUpgrade(Upgrade* up);

	void applyCombo(int b);

	//The bullet's graze "radar" has collided with Player.
	void onGrazeTouch(Bullet* bullet);
	//Effect is applied after the graze "radar" loses contact.
	void onGrazeCleared(Bullet* bullet);
protected:
	void applyGraze(int p);
	void startRespawn();
	void applyRespawn();

	void setHudEffect(Attribute id, Attribute max_id);
	void setHudEffect(Attribute id, float maxVal);
	void updateHudAttribute(Attribute id);

	bool canPlaceBomb(SpaceVect pos);

	SpaceVect respawnPos;
	SpaceFloat respawnAngle;
	SpaceFloat respawnTimer = 0.0;
	SpaceFloat respawnMaskTimer = 0.0;

	unordered_set<Bullet*> grazeContacts;

	float lookModeHoldTimer = 0.0f;
	float interactCooldown = 0.0f;
	float bombCooldown = 0.0f;

	vector<shared_ptr<FirePattern>> firePatterns;
	vector<shared_ptr<SpellDesc>> powerAttacks;
	vector<shared_ptr<SpellDesc>> spells;
	int firePatternIdx = -1;
	int powerAttackIdx = -1;
	int spellIdx = -1;

	PlayScene* playScene = nullptr;

	bool isAutoLook = false;
	bool isAutoLookToggled = false;
	bool isAutoFire = false;
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
	FlandrePC(GSpace* space, ObjectIDType id, const SpaceVect& pos, Direction d);

	virtual inline string imageSpritePath() const { return "sprites/flandre.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 128; }

	virtual CircleLightArea getLightSource() const;
};

class RumiaPC : virtual public Player, public BaseAttributes<RumiaPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(RumiaPC);

	virtual inline string imageSpritePath() const { return "sprites/rumia.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 128; }

	virtual CircleLightArea getLightSource() const;
};

class CirnoPC : virtual public Player, public BaseAttributes<CirnoPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(CirnoPC);
	
	virtual inline string imageSpritePath() const { return "sprites/cirno.png"; }
	virtual CircleLightArea getLightSource() const;
};

#endif /* Player_hpp */
