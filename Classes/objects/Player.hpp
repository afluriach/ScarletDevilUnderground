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

class Player : public Agent
{
public:
	static const float centerLookHoldThresh;
    static const float interactCooldownTime;
	static const float bombCooldownTime;
	static const float spellCooldownTime;
    static const float hitFlickerInterval;

	static const SpaceFloat sprintSpeedRatio;
	static const SpaceFloat sprintTime;
	static const SpaceFloat sprintCooldownTime;
	static const SpaceFloat focusSpeedRatio;
	static const SpaceFloat bombThrowSpeed;
	
	static const SpaceFloat interactDistance;
	static const SpaceFloat grazeRadius;

	static const float sprintCost;

	Player(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<agent_properties> props
	);
	~Player();
    
	virtual void onRemove();
	virtual void setCrntRoom(RoomSensor* room);

	void equipFirePatterns();
	void equipSpells();
	void equipPowerAttacks();
	void equipItems();

    //setting for player object sensing
	inline virtual SpaceFloat getRadarRadius() const { return grazeRadius; }
    inline virtual SpaceFloat getDefaultFovAngle() const { return 0.0;}

	virtual void onBulletHitTarget(Bullet* bullet, Agent* target);
    virtual bool hit(DamageInfo damage, SpaceVect n);
    
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

	virtual AttributeMap getAttributeUpgrades() const;
    
	void useDoor(Door* interactTarget);
	void moveToDestinationDoor(Door* dest);
	void applyUpgrade(Attribute attr, float val);

	void applyCombo(int b);
	void gameOver();
protected:
	SpaceVect sprintDirection;
	SpaceFloat sprintTimer = 0.0;

	float interactCooldown = 0.0f;
	float bombCooldown = 0.0f;
	float spellCooldown = 0.0f;
	unsigned int crntSpell = 0;

	vector<local_shared_ptr<FirePattern>> firePatterns;
	vector<const SpellDesc*> powerAttacks;
	vector<const SpellDesc*> spells;
	int firePatternIdx = -1;
	int powerAttackIdx = -1;
	int spellIdx = -1;

	PlayScene* playScene = nullptr;

	//Override spell cooldown
	bool isPowerAttack = false;
	bool isAutoFire = false;
	bool isSprintActive = false;
	bool isFocusActive = false;
	bool isComboActive = false;
};

#endif /* Player_hpp */
