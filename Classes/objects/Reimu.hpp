//
//  Reimu.hpp
//  Koumachika
//
//  Created by Toni on 3/15/16.
//
//

#ifndef Reimu_h
#define Reimu_h

#include "Enemy.hpp"

class Reimu : virtual public Agent, public DialogEntity, public NoAttributes
{
public:
	MapObjCons(Reimu);
    
    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::npc;}
    
    inline string imageSpritePath() const {return "sprites/reimu.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    inline virtual bool isDialogAvailable(){ return true;}
    inline virtual string getDialog(){
        return "dialogs/warning_about_rumia";
    }
};

class ReimuEnemy : public Enemy, public BaseAttributes<ReimuEnemy>
{
public:
	static constexpr int orbCount = 4;
	static const AttributeMap baseAttributes;

	MapObjCons(ReimuEnemy);

	void lockDoors();
	void unlockDoors();
	void spawnOrbs();
	void removeOrbs();

	virtual void onZeroHP();

	virtual inline SpaceFloat getRadarRadius() const { return 5.0; }
	virtual GType getRadarType() const { return GType::enemySensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	virtual inline SpaceFloat getMass() const { return 40.0; }
	virtual inline GType getType() const { return GType::enemy; }

	virtual inline string imageSpritePath() const { return "sprites/reimu.png"; }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& fsm);
protected:
	string activations;
	array<gobject_ref, orbCount> orbs;
};

class ReimuMain : public ai::Function
{
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	virtual void onExit(ai::StateMachine& sm);
	FuncGetName(ReimuMain)
};

#endif /* Reimu_h */
