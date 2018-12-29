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
#include "scenes.h"

class Collectible;
class ControlState;
class FirePattern;
class SpellDesc;
class PlayScene;

class Player :
virtual public Agent,
public RegisterUpdate<Player>
{
public:
    static const float interactCooldownTime;
    static const float hitFlickerInterval;

	MapObjCons(Player);
    
	virtual void onPitfall();

	virtual void setFirePatterns() = 0;
	virtual void equipSpells() = 0;
    
    //setting for player object sensing
	inline virtual SpaceFloat getRadarRadius() const { return 2.5; }
	inline virtual GType getRadarType() const { return GType::objectSensor; }
    inline virtual SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0;}

	bool isProtected() const;
	void setProtection();
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
    
	void init();
    void update();
    void updateHitTime();

	virtual void onZeroHP();

	void checkMovementControls(const ControlInfo& cs);
	void checkFireControls(const ControlInfo& cs);
	void checkItemInteraction(const ControlInfo& cs);
    void updateSpellControls(const ControlInfo& cs);
	void onSpellStop();

	void applyAttributeModifier(Attribute id, float val);
    
	FirePattern* getFirePattern();

	bool trySetFirePattern(int idx);
	bool trySetFirePatternNext();
	bool trySetFirePatternPrevious();

    void onCollectible(Collectible* coll);
	void applyGraze(int p);
protected:
	void setHudEffect(Attribute id, Attribute max_id);
	void updateHudAttribute(Attribute id);

	template<typename...Args>
	inline pair<function<void(void)>, GScene::updateOrder> make_hud_action(void (HUD::*m)(Args...), Args ...args)
	{
		return pair<function<void(void)>, GScene::updateOrder>(generate_action(playScene->hud, m, args...), GScene::updateOrder::hudUpdate);
	}

	boost::rational<int> interactCooldown = 0;

	vector<unique_ptr<FirePattern>> firePatterns;
	int crntFirePattern = 0;

	SpellDesc* equippedSpell = nullptr;

	PlayScene* playScene = nullptr;

	bool suppressFiring = false;
	bool suppressMovement = false;
};

class FlandrePC : virtual public Player, public BaseAttributes<FlandrePC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(FlandrePC);

	virtual inline string imageSpritePath() const { return "sprites/flandre/"; }
	virtual inline int pixelWidth() const { return 256; }

	virtual void setFirePatterns();
	virtual void equipSpells();
};

class RumiaPC : virtual public Player, public BaseAttributes<RumiaPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(RumiaPC);

	virtual inline string imageSpritePath() const { return "sprites/rumia/"; }
	virtual inline int pixelWidth() const { return 256; }

	virtual void setFirePatterns();
	virtual void equipSpells();
};

class CirnoPC : virtual public Player, public BaseAttributes<CirnoPC>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(CirnoPC);
	
	virtual inline string imageSpritePath() const { return "sprites/cirno.png"; }
	virtual void setFirePatterns();
	virtual void equipSpells();
};

#endif /* Player_hpp */
