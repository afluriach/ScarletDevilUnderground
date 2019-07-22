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

class Collectible;
struct ControlInfo;
class ControlState;
class Door;
class EnemyBullet;
class FirePattern;
class SpellDesc;
class PlayScene;
class Upgrade;

class Player : public Agent
{
public:
	static const float centerLookHoldThresh;
    static const float interactCooldownTime;
	static const float bombCooldownTime;
    static const float hitFlickerInterval;

	static const SpaceFloat sprintSpeedRatio;
	static const SpaceFloat sprintTime;
	static const SpaceFloat sprintCooldownTime;
	static const SpaceFloat focusSpeedRatio;
	static const SpaceFloat bombThrowSpeed;
	
	static const SpaceFloat interactDistance;
	static const SpaceFloat grazeRadius;

	static const float bombCost;
	static const float sprintCost;

	Player(GSpace* space, ObjectIDType id, const SpaceVect& pos, Direction d);
	Player(
		GSpace* space, ObjectIDType id, const ValueMap& args,
		const string& attributes
	);
	inline virtual ~Player() {}
    
	virtual void onPitfall();
	virtual void setCrntRoom(RoomSensor* room);

	void equipFirePatterns();
	void equipSpells();
	void equipPowerAttacks();

    //setting for player object sensing
	inline virtual SpaceFloat getRadarRadius() const { return grazeRadius; }
	inline virtual GType getRadarType() const { return GType::playerGrazeRadar; }
    inline virtual SpaceFloat getDefaultFovAngle() const { return 0.0;}

	virtual void onDetect(GObject* other);
	virtual void onEndDetect(GObject* other);

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

    virtual inline GType getType() const {return GType::player;}
    
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
	SpaceVect getInteractFeeler() const;

	virtual void init();
    virtual void update();
	void updateCombo();

	virtual void onZeroHP();

	//Since the Player is the sound source, override this to avoid playing zero-distance sounds.
	virtual ALuint playSoundSpatial(
		const string& path,
		float volume = 1.0f,
		bool loop = false,
		float yPos = 0.0f
	);

	void checkMovementControls(const ControlInfo& cs);
	void checkFireControls(const ControlInfo& cs);
	void checkBombControls(const ControlInfo& cs);
	void checkItemInteraction(const ControlInfo& cs);
    void updateSpellControls(const ControlInfo& cs);
	void onSpellStop();

	SpaceFloat getSpeedMultiplier();
	void setFocusMode(bool b);

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

	bool canPlaceBomb(SpaceVect pos);

	SpaceVect sprintDirection;
	SpaceVect respawnPos;
	SpaceFloat respawnAngle;
	SpaceFloat respawnTimer = 0.0;
	SpaceFloat respawnMaskTimer = 0.0;
	SpaceFloat sprintTimer = 0.0;

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

	//Override spell cooldown
	bool isPowerAttack = false;
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

class FlandrePC : public Player
{
public:
	static const string baseAttributes;

	MapObjCons(FlandrePC);
	FlandrePC(GSpace* space, ObjectIDType id, const SpaceVect& pos, Direction d);

	virtual inline string getSprite() const { return "flandre"; }

	virtual shared_ptr<LightArea> getLightSource() const;
};

class RumiaPC : public Player
{
public:
	static const string baseAttributes;

	MapObjCons(RumiaPC);

	virtual inline string getSprite() const { return "rumia"; }

	virtual shared_ptr<LightArea> getLightSource() const;
};

class CirnoPC : public Player
{
public:
	static const string baseAttributes;

	MapObjCons(CirnoPC);
	
	virtual inline string getSprite() const { return "cirno"; }
	virtual shared_ptr<LightArea> getLightSource() const;
};

#endif /* Player_hpp */
