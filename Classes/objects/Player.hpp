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
    static const float hitFlickerInterval;
	
	static const SpaceFloat interactDistance;
	static const SpaceFloat grazeRadius;

	Player(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<agent_properties> props
	);
	~Player();
    
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

	virtual void onZeroHP();
    virtual void onPitfall();

	//Since the Player is the sound source, override this to avoid playing zero-distance sounds.
	virtual ALuint playSoundSpatial(
		const string& path,
		float volume = 1.0f,
		bool loop = false,
		float yPos = 0.0f
	);

	virtual AttributeMap getAttributeUpgrades() const;
    
	void useDoor(Door* interactTarget);
	void moveToDestinationDoor(Door* dest);
	void applyUpgrade(Attribute attr, float val);

	void applyCombo(int b);
	void gameOver();
protected:
	PlayScene* playScene = nullptr;

	bool isComboActive = false;
};

#endif /* Player_hpp */
